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


    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      int function_num = 0;
      int instruction_num = 0;

      for (auto &B : F) {
        function_num = function_num + 1;
        for (auto &I : B) {
          instruction_num = instruction_num +1;
          MDNode *metadata = I.getMetadata("dbg");
          DILocation *debugLocation = dyn_cast<DILocation>(metadata);
          const DebugLoc &debugLoc = DebugLoc(debugLocation);
          if (!(function_num == FunctionId and instruction_num == InstructionId and debugLocation->getFilename() == InputFileName)){
            continue;
          }
         
          if (auto *op = dyn_cast<BranchInst>(&I)) {
            if (op->isConditional() == true)
            {
              errs() << "old op" << *op << "\n";
              op->swapSuccessors();
              errs() << "new op" << *op << "\n";
              
            }
           
            // We modified the code.
            return true; 
          }
        }
      }

       return false;}
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("Branch", "Switching branches", false, false);
