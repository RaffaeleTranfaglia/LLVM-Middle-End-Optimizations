#ifndef LLVM_TRANSFORMS_LOOPOPTS_H
#define LLVM_TRANSFORMS_LOOPOPTS_H

#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

namespace llvm
{
    class LoopOpts : public PassInfoMixin<LoopOpts>
    {
        public:
        PreservedAnalyses run (Loop &L, LoopAnalysisManager &LAM, 
                                LoopStandardAnalysisResults &LAR, LPMUpdater &LU);
    };
}

#endif // LLVM_TRANSFORMS_LOOPOPTS_H