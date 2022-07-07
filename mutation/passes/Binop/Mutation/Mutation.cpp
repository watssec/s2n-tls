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
static cl::opt<std::string> TargetType("target_type", cl::desc("Specify the target type of mutation"), cl::value_desc("target type"));

namespace {


  struct SkeletonPass : public ModulePass {
    static char ID;


    SkeletonPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {    
    int function_num = 0;
    int instruction_num = 0;
    for (auto &F: M){
      function_num = function_num + 1;
      for (auto &B : F) {
        
        for (auto &I : B) {
    
          instruction_num = instruction_num +1;
          if (!(function_num == FunctionId and instruction_num == InstructionId)){
            continue;
          }
        errs() << "target_type" << TargetType << "\n";
          if (auto *op = dyn_cast<BinaryOperator>(&I)) {
            // Insert at the point where the instruction `op` appears.
            IRBuilder<> builder(op);
	          
            // Make a multiply with the same operands as `op`.
            Value *lhs = op->getOperand(0);
            Value *rhs = op->getOperand(1);
            Value *newinst = builder.CreateMul(lhs, rhs);
            
            if(TargetType == "add"){
                newinst = builder.CreateAdd(lhs,rhs);
            }else if(TargetType == "sub")
             {                
                newinst = builder.CreateSub(lhs,rhs);
            }else if(TargetType == "fadd")
            {  
                newinst = builder.CreateFAdd(lhs, rhs);
            }else if(TargetType == "fsub")
            {
                newinst = builder.CreateFSub(lhs, rhs);
            }else if(TargetType == "fmul")
            {
                newinst = builder.CreateFMul(lhs, rhs);
            }else if(TargetType == "udiv")
            { 
                newinst = builder.CreateUDiv(lhs, rhs);
            }else if(TargetType == "urem")
            {
                newinst = builder.CreateURem(lhs, rhs);
            }else if(TargetType == "fdiv"){
                newinst = builder.CreateFDiv(lhs, rhs);
            }else if(TargetType == "frem"){
                newinst = builder.CreateFRem(lhs, rhs);
            }else if(TargetType == "sdiv"){
                newinst = builder.CreateSDiv(lhs, rhs);
            }else if(TargetType == "srem"){
                newinst = builder.CreateSRem(lhs, rhs);
            }else if(TargetType == "shl"){
                newinst = builder.CreateShl(lhs, rhs);
            }else if(TargetType == "lshr"){
                newinst = builder.CreateLShr(lhs, rhs);
            }else if(TargetType == "ashr"){
                newinst = builder.CreateAShr(lhs, rhs);
            }else if(TargetType == "and"){
                newinst = builder.CreateAnd(lhs, rhs);
            }else if(TargetType == "or"){
                newinst = builder.CreateOr(lhs, rhs);
            }else if(TargetType == "xor"){
                newinst = builder.CreateXor(lhs, rhs);
            } 
            errs() << "op" << *op << "\n";
            // Everywhere the old instruction was used as an operand, use our
            // new multiply instruction instead.
            for (auto &U : op->uses()) {
              User *user = U.getUser();
              // A User is anything with operands.
              user->setOperand(U.getOperandNo(), newinst);
           
            }

            // We modified the code.
            return true; 
          }
        }
        }
      }

       return false;}
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("Binop", "Binary operator mutation", false, false);
