#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
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


  struct SkeletonPass : public ModulePass {
    static char ID;


    SkeletonPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      int function_num = 0;
      int instruction_num = 0;

      for (auto &F : M){
        function_num = function_num + 1;
        for (auto &B : F) {
          for (auto &I : B) {
            instruction_num = instruction_num +1;
            MDNode *metadata = I.getMetadata("dbg");
            DILocation *debugLocation = dyn_cast<DILocation>(metadata);
            if(metadata == 0x0){
              continue;
            }
            
            if (!(function_num == FunctionId and instruction_num == InstructionId)){
              continue;
            }

            if (auto *op = dyn_cast<ICmpInst>(&I)) {
              IRBuilder<> builder(op);
              // Make a multiply with the same operands as `op`.
              CmpInst::Predicate predicate= op->getPredicate();

              Value *lhs = op->getOperand(0);
              Value *rhs = op->getOperand(1);
              errs() << "oldinst" << *op << "\n";
              Value *newinst = builder.CreateICmp(predicate, rhs, lhs, "");
              for (auto &U: op->uses()) {
                User *user = U.getUser();
                user->setOperand(U.getOperandNo(),newinst);
              }
              errs() << "newinst" << *newinst << "\n";
              return true;
            // op->swapOperands();
          
              // Everywhere the old instruction was used as an operand, use our
              // new multiply instruction instead.


              // We modified the code.
            }
          }
        }
      }
      return false;  }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("ICmp", "Icmp operator mutation", false, false);
