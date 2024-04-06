#ifndef LLVM_TRANSFORMS_LOCALOPTS_H
#define LLVM_TRANSFORMS_LOCALOPTS_H

#include "llvm/IR/PassManager.h"
#include <llvm/IR/Constants.h>

namespace llvm 
{
    class LocalOpts : public PassInfoMixin<LocalOpts> {
    public:
        PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
    };
} // namespace llvm
#endif // LLVM_TRANSFORMS_LOCALOPTS_H

