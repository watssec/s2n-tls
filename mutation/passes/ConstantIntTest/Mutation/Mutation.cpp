#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Instructions.h"
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
static cl::opt<int> OperandId("operand_num", cl::desc("Specify operand number"), cl::value_desc("operand number"));
static cl::opt<std::string> TargetType("target_type", cl::desc("Specify target type"), cl::value_desc("target type"));


namespace {
  struct SkeletonPass : public ModulePass {
    static char ID;
    int function_num = 0;
    int instruction_num = 0;

    SkeletonPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      errs() << "functionID" << FunctionId << "\n";
    for (auto &F: M){
     
      function_num = function_num + 1; 
      for (auto &B : F){
        for (auto &I : B) {
          int operand_num = 0;
          instruction_num = instruction_num +1;
  	  if (auto *inst = dyn_cast<IntrinsicInst>(&I)){
	    errs() << "inst"<< *inst << "\n";
	  }       
	  for(Use &U: I.operands())
            {
  
              operand_num = operand_num +1;
	      Value *V = U.get();
	     
              if(!(function_num == FunctionId and instruction_num == InstructionId)){
                continue;
              }
              errs() << "instruction_num" << instruction_num << "\n";
              errs() << "I" << I << "\n";
              errs() << "old V" << *V << "\n";

              if(auto *func = dyn_cast<Function>(V)){
                errs() << "func" << func << "\n";
                errs() << "func is intrinsic or not " << func->isIntrinsic() << "\n";
              }

	      if(auto *op = dyn_cast<ConstantInt>(V)){
              int value = op->getZExtValue();
	     
              if (TargetType == "0"){
                value = 0;
              }else if(TargetType == "1"){
                value = 1;
              }else if(TargetType == "2"){
                value = -1;
              }else if(TargetType == "3"){
                value = value + 1;
              }else if(TargetType == "4"){
                value = value - 1;
              }
              Value *newvalue = ConstantInt::get(op->getType(),value);
              errs() << "new value" << *newvalue << "\n";
              I.setOperand(U.getOperandNo(),newvalue);
            
             
                }
            }
            }
        }
    }
      return true;  
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("ConstantInt", "Constant mutation", false, false);
