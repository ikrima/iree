// Copyright 2021 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "iree/compiler/Conversion/CodegenUtils/FunctionUtils.h"
#include "iree/compiler/Conversion/LinalgToLLVMGPU/ConvertToLLVM.h"
#include "iree/compiler/Conversion/LinalgToLLVMGPU/Passes.h"
#include "iree/compiler/Dialect/IREE/IR/IREEOps.h"
#include "mlir/Conversion/GPUToROCDL/GPUToROCDLPass.h"
#include "mlir/Conversion/StandardToLLVM/ConvertStandardToLLVM.h"
#include "mlir/Conversion/VectorToLLVM/ConvertVectorToLLVM.h"
#include "mlir/Dialect/GPU/Passes.h"
#include "mlir/Dialect/LLVMIR/ROCDLDialect.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/Dialect/Vector/VectorOps.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/Passes.h"

namespace mlir {
namespace iree_compiler {

namespace {

/// A pass that replaces all occurrences of GPU device operations with their
/// corresponding ROCDL equivalent.
///
/// This pass only handles device code and is not meant to be run on GPU host
/// code.
struct ConvertToROCDLPass
    : public PassWrapper<ConvertToROCDLPass, OperationPass<ModuleOp>> {
  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<LLVM::LLVMDialect, ROCDL::ROCDLDialect>();
  }
  void runOnOperation() override {
    ModuleOp m = getOperation();

    /// Customize the bitwidth used for the device side index computations.
    LowerToLLVMOptions options(m.getContext(), DataLayout(m));
    options.overrideIndexBitwidth(64);
    LLVMTypeConverter converter(m.getContext(), options);
    // Apply in-dialect lowering first. In-dialect lowering will replace ops
    // which need to be lowered further, which is not supported by a single
    // conversion pass.
    // Run Vector -> Vector transformations ahead of conversion to LLVM.
    {
      OwningRewritePatternList patterns(&getContext());
      populateScalarizeMathOps(patterns);
      vector::populateVectorToVectorCanonicalizationPatterns(patterns);
      vector::populateVectorSlicesLoweringPatterns(patterns);
      vector::populateVectorContractLoweringPatterns(
          patterns,
          vector::VectorTransformsOptions().setVectorTransformsOptions(
              vector::VectorContractLowering::OuterProduct));
      mlir::vector::populateVectorTransposeLoweringPatterns(patterns);
      (void)applyPatternsAndFoldGreedily(m, std::move(patterns));
    }
    {
      OwningRewritePatternList patterns(&getContext());
      populateGpuRewritePatterns(patterns);
      (void)applyPatternsAndFoldGreedily(m, std::move(patterns));
    }
    {
      OwningRewritePatternList llvmPatterns(&getContext());
      populateLLVMConversionPatterns(&getContext(), llvmPatterns, converter,
                                     true);
      populateStdToLLVMConversionPatterns(converter, llvmPatterns);
      populateVectorToLLVMConversionPatterns(converter, llvmPatterns);
      populateGpuToROCDLConversionPatterns(converter, llvmPatterns);
      LLVMConversionTarget target(getContext());
      populateStdToLLVMFuncOpConversionPattern(converter, llvmPatterns);
      configureGpuToROCDLConversionLegality(target);
      target.addDynamicallyLegalOp<FuncOp>([&](FuncOp funcOp) {
        if (isEntryPoint(funcOp)) return false;
        return true;
      });
      if (failed(applyPartialConversion(m, target, std::move(llvmPatterns))))
        signalPassFailure();
    }
  }
};

}  // anonymous namespace

std::unique_ptr<OperationPass<ModuleOp>> createConvertToROCDLPass() {
  return std::make_unique<ConvertToROCDLPass>();
}

static PassRegistration<ConvertToROCDLPass> pass(
    "iree-codegen-convert-to-rocdl",
    "Perform final conversion from builtin/GPU/HAL/standard dialect to LLVM "
    "and ROCDL dialects");

}  // namespace iree_compiler
}  // namespace mlir