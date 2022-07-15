
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <map>
#include "nlohmann/json.hpp"
#include <iostream>
#include <fstream>
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "string"
#include "cstring"

using namespace llvm;
using json = nlohmann::json;


namespace {
  
    json JsonDump(Function &F, Instruction &I, const char *Opcode, json mutation_map,
    int instruction_num, int function_num, int operand_num, char *mutation_type){
    json mutation_object = json::object();
    MDNode *metadata = I.getMetadata("dbg");
    if(metadata == 0x0){
      return mutation_map;
    }
    DILocation *debugLocation = dyn_cast<DILocation>(metadata);
    const DebugLoc &debugLoc = DebugLoc(debugLocation);

    mutation_object["file_name"] = debugLocation->getFilename();
    mutation_object["function_name"] = F.getName();  
    mutation_object["instruction_line"] = debugLocation->getLine();
    mutation_object["opcode"] = Opcode;
    mutation_object["instruction_col"] = debugLoc.getCol();
    mutation_object["instruction_num"] = instruction_num;
    mutation_object["function_num"] = function_num;
    mutation_object["operand_num"] = operand_num;
    mutation_object["mutation_type"] = mutation_type;
    mutation_map.push_back(mutation_object);
    return mutation_map;
    }
  
  
  struct SkeletonPass : public ModulePass {
    static char ID;
    
    SkeletonPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
    int function_num = 0;
    int instruction_num = 0;
    for (auto &F: M){
     
        function_num = function_num + 1; 
        for (auto &B : F){
        for (auto &I : B) {
          
	  instruction_num = instruction_num +1;
	  json mutation_map; 
	  const char *eqopcode = "eq";
          std::ifstream i("genesis_info.json");
          if (i.good()){
            i >> mutation_map;
          }else{
            mutation_map = json::array();
          }

          if (auto *inst = dyn_cast<IntrinsicInst>(&I)){
            continue;
          }

          // opcode mutation
          int operand_num = 0;
          if (auto *op = dyn_cast<BinaryOperator>(&I)) {
              const char *OpCode = op->getOpcodeName();
	      char *mutation_type = "Binop";
            mutation_map = JsonDump(F, I, OpCode, mutation_map, instruction_num, function_num, operand_num, mutation_type);
            std::ofstream o ("genesis_info.json", std::ofstream::trunc);
            o << std::setw(4) << mutation_map << std::endl;
          }
          
          if (auto *op = dyn_cast<ICmpInst>(&I)) {
              const char *OpCode = op->getOpcodeName();
	      if (*OpCode == *eqopcode){
	      continue;}
              char *mutation_type = "ICmp";
              mutation_map = JsonDump(F, I, OpCode, mutation_map, instruction_num, function_num, operand_num, mutation_type);
              std::ofstream o ("genesis_info.json", std::ofstream::trunc);
              o << std::setw(4) << mutation_map << std::endl;
          }

          if (auto *op = dyn_cast<FCmpInst>(&I)) {
              const char *OpCode = op->getOpcodeName();
	      if (*OpCode == *eqopcode){
	      continue;}
              char *mutation_type = "FCmp";
              mutation_map = JsonDump(F, I, OpCode, mutation_map, instruction_num, function_num, operand_num, mutation_type);
              std::ofstream o ("genesis_info.json", std::ofstream::trunc);
              o << std::setw(4) << mutation_map << std::endl;
          }
          if (auto *op = dyn_cast<BranchInst>(&I)) {
              if (op->isConditional() == true){
              const char *OpCode = op-> getOpcodeName();
              char *mutation_type = "Branch";
              mutation_map = JsonDump(F, I, OpCode, mutation_map, instruction_num, function_num, operand_num, mutation_type);
              std::ofstream o ("genesis_info.json", std::ofstream::trunc);
              o << std::setw(4) << mutation_map << std::endl;
            }
          }
	  if (auto *op = dyn_cast<GetElementPtrInst>(&I)){
	      continue;
	  }
            for(Use &U: I.operands())
            {
              operand_num = operand_num + 1;
              Value *V = U.get();
	      if(auto *func = dyn_cast<Function>(V)){
	        if(func->isIntrinsic()){
		  continue;
		} 
	      }
             
	      if(auto *op = dyn_cast<ConstantInt>(V)){
              std::string type_str;
              llvm::raw_string_ostream rso(type_str);
              op->getType()->print(rso);
              char *mutation_type = "ConstantInt";
              const char * OpCode = rso.str().c_str();
              mutation_map = JsonDump(F, I, OpCode, mutation_map, instruction_num, function_num, operand_num, mutation_type);
              std::ofstream o ("genesis_info.json", std::ofstream::trunc);
              o << std::setw(4) << mutation_map << std::endl;
            
              }

              if(auto *op = dyn_cast<ConstantFP>(V)){
              std::string type_str;
              llvm::raw_string_ostream rso(type_str);
              op->getType()->print(rso);
              char *mutation_type = "ConstantFP";
              const char * OpCode = rso.str().c_str();
              mutation_map = JsonDump(F, I, OpCode, mutation_map, instruction_num, function_num, operand_num, mutation_type);
              std::ofstream o ("genesis_info.json", std::ofstream::trunc);
              o << std::setw(4) << mutation_map << std::endl;
            
              }
            }
        }
        }
      }
      return false;
    }
  };
}

char SkeletonPass::ID = 0;
static RegisterPass<SkeletonPass> X("Initialization", "Scanning out all the possible mutation points", false, false);

