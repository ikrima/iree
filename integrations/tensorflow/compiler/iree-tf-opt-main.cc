// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Main entry function for iree-tf-opt and derived binaries.
//
// Based on iree-opt with the addition of TF dialects and passes

#include "integrations/tensorflow/compiler/dialect/tf_strings/ir/dialect.h"
#include "integrations/tensorflow/compiler/dialect/tf_tensorlist/ir/tf_tensorlist_dialect.h"
#include "iree/compiler/Conversion/HLOToLinalg/Passes.h"
#include "iree/compiler/Conversion/init_conversions.h"
#include "iree/compiler/Dialect/HAL/Conversion/Passes.h"
#include "iree/tools/init_compiler_modules.h"
#include "iree/tools/init_iree_dialects.h"
#include "iree/tools/init_iree_passes.h"
#include "iree/tools/init_mlir_dialects.h"
#include "iree/tools/init_mlir_passes.h"
#include "iree/tools/init_targets.h"
#include "iree/tools/init_xla_dialects.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "mlir/IR/AsmState.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Support/MlirOptMain.h"
#include "tensorflow/compiler/mlir/tensorflow/ir/tf_device.h"
#include "tensorflow/compiler/mlir/tensorflow/ir/tf_executor.h"
#include "tensorflow/compiler/mlir/tensorflow/ir/tf_ops.h"
#include "tensorflow/compiler/mlir/tensorflow/ir/tf_saved_model.h"

#ifdef IREE_HAVE_EMITC_DIALECT
#include "emitc/InitDialect.h"
#endif  // IREE_HAVE_EMITC_DIALECT

static llvm::cl::opt<std::string> inputFilename(llvm::cl::Positional,
                                                llvm::cl::desc("<input file>"),
                                                llvm::cl::init("-"));

static llvm::cl::opt<std::string> outputFilename(
    "o", llvm::cl::desc("Output filename"), llvm::cl::value_desc("filename"),
    llvm::cl::init("-"));

static llvm::cl::opt<bool> splitInputFile(
    "split-input-file",
    llvm::cl::desc("Split the input file into pieces and process each "
                   "chunk independently"),
    llvm::cl::init(false));

static llvm::cl::opt<bool> verifyDiagnostics(
    "verify-diagnostics",
    llvm::cl::desc("Check that emitted diagnostics match "
                   "expected-* lines on the corresponding line"),
    llvm::cl::init(false));

static llvm::cl::opt<bool> verifyPasses(
    "verify-each",
    llvm::cl::desc("Run the verifier after each transformation pass"),
    llvm::cl::init(true));

static llvm::cl::opt<bool> allowUnregisteredDialects(
    "allow-unregistered-dialect",
    llvm::cl::desc("Allow operation with no registered dialects"),
    llvm::cl::init(true));

static llvm::cl::opt<bool> showDialects(
    "show-dialects", llvm::cl::desc("Print the list of registered dialects"),
    llvm::cl::init(false));

void registerTFDialects(mlir::DialectRegistry &registry) {
  registry.insert<mlir::TF::TensorFlowDialect,
                  mlir::tf_executor::TensorFlowExecutorDialect,
                  mlir::tf_device::TensorFlowDeviceDialect,
                  mlir::tf_saved_model::TensorFlowSavedModelDialect>();
}

void registerExtensionDialects(mlir::DialectRegistry &registry) {
  registry.insert<mlir::iree_compiler::tf_strings::TFStringsDialect,
                  mlir::tf_tensorlist::TfTensorListDialect>();
}

int main(int argc, char **argv) {
  // TODO(#2958): There's a lot of duplication with iree-opt here. Factor out
  // the common functionality.
  llvm::InitLLVM y(argc, argv);

  mlir::DialectRegistry registry;
  mlir::registerMlirDialects(registry);
  mlir::registerMlirPasses();
#ifdef IREE_HAVE_EMITC_DIALECT
  mlir::registerEmitCDialect(registry);
#endif  // IREE_HAVE_EMITC_DIALECT
  mlir::registerXLADialects(registry);
  mlir::iree_compiler::registerIreeDialects(registry);
  mlir::iree_compiler::registerIreeCompilerModuleDialects(registry);
  registerTFDialects(registry);
  registerExtensionDialects(registry);

  mlir::iree_compiler::registerAllIreePasses();
  mlir::iree_compiler::registerHALConversionPasses();
  mlir::iree_compiler::registerHALTargetBackends();
  mlir::iree_compiler::registerLinalgToSPIRVPasses();
  mlir::iree_compiler::registerHLOToLinalgPasses();
  mlir::iree_compiler::registerLinalgToLLVMPasses();

  // Register MLIRContext command-line options like
  // -mlir-print-op-on-diagnostic.
  mlir::registerMLIRContextCLOptions();
  // Register assembly printer command-line options like
  // -mlir-print-op-generic.
  mlir::registerAsmPrinterCLOptions();
  // Register pass manager command-line options like -print-ir-*.
  mlir::registerPassManagerCLOptions();

  mlir::PassPipelineCLParser passPipeline("", "Compiler passes to run");

  // Parse pass names in main to ensure static initialization completed.
  llvm::cl::ParseCommandLineOptions(argc, argv,
                                    "IREE modular optimizer driver\n");

  if (showDialects) {
    llvm::outs() << "Available Dialects:\n";
    interleave(
        registry, llvm::outs(),
        [](auto &registryEntry) { llvm::outs() << registryEntry.first; }, "\n");
    return 0;
  }

  // Set up the input file.
  std::string errorMessage;
  auto file = mlir::openInputFile(inputFilename, &errorMessage);
  if (!file) {
    llvm::errs() << errorMessage << "\n";
    return 1;
  }

  auto output = mlir::openOutputFile(outputFilename, &errorMessage);
  if (!output) {
    llvm::errs() << errorMessage << "\n";
    exit(1);
  }

  if (failed(mlir::MlirOptMain(output->os(), std::move(file), passPipeline,
                               registry, splitInputFile, verifyDiagnostics,
                               verifyPasses, allowUnregisteredDialects,
                               /*preloadDialectsInContext=*/false))) {
    return 1;
  }
}
