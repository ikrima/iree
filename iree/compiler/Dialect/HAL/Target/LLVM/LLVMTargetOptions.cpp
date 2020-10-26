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

#include "iree/compiler/Dialect/HAL/Target/LLVM/LLVMTargetOptions.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Host.h"
#include "llvm/Target/TargetOptions.h"

namespace mlir {
namespace iree_compiler {
namespace IREE {
namespace HAL {

LLVMTargetOptions getDefaultLLVMTargetOptions() {
  LLVMTargetOptions targetOptions;

  // Host target triple.
  targetOptions.targetTriple = llvm::sys::getDefaultTargetTriple();
  targetOptions.targetCPU = llvm::sys::getHostCPUName().str();
  {
    llvm::SubtargetFeatures features;
    llvm::StringMap<bool> hostFeatures;
    if (llvm::sys::getHostCPUFeatures(hostFeatures)) {
      for (auto &f : hostFeatures) {
        features.AddFeature(f.first(), f.second);
      }
    }
    targetOptions.targetCPUFeatures = features.getString();
  }

  // LLVM loop optimization options.
  targetOptions.pipelineTuningOptions.LoopInterleaving = true;
  targetOptions.pipelineTuningOptions.LoopVectorization = true;
  targetOptions.pipelineTuningOptions.LoopUnrolling = true;

  // LLVM SLP Auto vectorizer.
  targetOptions.pipelineTuningOptions.SLPVectorization = true;

  // LLVM -O3.
  targetOptions.optLevel = llvm::PassBuilder::OptimizationLevel::O3;
  targetOptions.options.FloatABIType = llvm::FloatABI::Hard;

  return targetOptions;
}

LLVMTargetOptions getLLVMTargetOptionsFromFlags() {
  auto llvmTargetOptions = getDefaultLLVMTargetOptions();

  static llvm::cl::opt<std::string> clTargetTriple(
      "iree-llvm-target-triple", llvm::cl::desc("LLVM target machine triple"),
      llvm::cl::init(llvmTargetOptions.targetTriple));
  static llvm::cl::opt<std::string> clTargetCPU(
      "iree-llvm-target-cpu", llvm::cl::desc("LLVM target machine CPU"),
      llvm::cl::init(llvmTargetOptions.targetCPU));
  static llvm::cl::opt<std::string> clTargetCPUFeatures(
      "iree-llvm-target-cpu-features",
      llvm::cl::desc("LLVM target machine CPU features"),
      llvm::cl::init(llvmTargetOptions.targetCPUFeatures));
  llvmTargetOptions.targetTriple = clTargetTriple;
  llvmTargetOptions.targetCPU = clTargetCPU;
  llvmTargetOptions.targetCPUFeatures = clTargetCPUFeatures;

  static llvm::cl::opt<llvm::FloatABI::ABIType> clTargetFloatABI(
      "iree-llvm-target-float-abi",
      llvm::cl::desc("LLVM target codegen enables soft float abi e.g "
                     "-mfloat-abi=softfp"),
      llvm::cl::init(llvmTargetOptions.options.FloatABIType),
      llvm::cl::values(
          clEnumValN(llvm::FloatABI::Default, "default", "Default (softfp)"),
          clEnumValN(llvm::FloatABI::Soft, "soft",
                     "Software floating-point emulation"),
          clEnumValN(llvm::FloatABI::Hard, "hard",
                     "Hardware floating-point instructions")));
  llvmTargetOptions.options.FloatABIType = clTargetFloatABI;

  static llvm::cl::opt<bool> clKeepArtifacts(
      "iree-llvm-keep-artifacts",
      llvm::cl::desc("Keep LLVM linker target artifacts (.so/.dll/etc)"),
      llvm::cl::init(llvmTargetOptions.keepArtifacts));
  llvmTargetOptions.keepArtifacts = clKeepArtifacts;

  return llvmTargetOptions;
}

}  // namespace HAL
}  // namespace IREE
}  // namespace iree_compiler
}  // namespace mlir
