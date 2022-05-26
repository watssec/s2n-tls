#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"


using namespace llvm;

static cl::opt<std::string> InputFileName("file_name", cl::desc("Specify input filename"), cl::value_desc("filename"));
static cl::opt<int> FunctionId("function_num", cl::desc("Specify function number"), cl::value_desc("function number"));
static cl::opt<int> InstructionId("instruction_num", cl::desc("Specify instruction number"), cl::value_desc("instruction number"));


namespace {


  struct SkeletonPass : public FunctionPass {
    static char ID;
    int function_num = 0;
    int instruction_num = 0;

    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      for(auto B = F.getBasicBlockList().begin(); B != F.getBasicBlockList().end(); B++)
        {
            for(auto inst = B->begin(); inst != B->end(); inst++)
            {
                for(Use &U:inst->operands())
                {
                    Value *V = U.get();
                    auto *op = dyn_cast<Constant>(V);
                }
            }
        }
      return true;  }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("Constant", "Constant mutation", false, false);
