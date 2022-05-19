
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
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


using namespace llvm;
using json = nlohmann::json;


namespace {
  
    json JsonDump(Function &F, Instruction &I, const char *Opcode, json mutation_map){
    json mutation_object = json::object();
    MDNode *metadata = I.getMetadata("dbg");
    DILocation *debugLocation = dyn_cast<DILocation>(metadata);
    const DebugLoc &debugLoc = DebugLoc(debugLocation);
    mutation_object["file_name"] = debugLocation->getFilename();
    mutation_object["function_name"] = F.getName();  
    mutation_object["instruction_line"] = debugLocation->getLine();
    mutation_object["instruction_type"] = Opcode;
    mutation_object["instruction_col"] = debugLoc.getCol();
    mutation_map.push_back(mutation_object);
    return mutation_map;
    }
  
  
  struct SkeletonPass : public FunctionPass {
    static char ID;
    
    SkeletonPass() : FunctionPass(ID) {}
    int function_num = 0;
    virtual bool runOnFunction(Function &F) {
      
      for (auto &B : F) {
		
        for (auto &I : B) {
	        json mutation_map; 
          std::ifstream i("genesis_info.json");
          if (i.good()){
            i >> mutation_map;
          }else{
            mutation_map = json::array();
          }
          if (auto *op = dyn_cast<BinaryOperator>(&I)) {
              const char *OpCode = op->getOpcodeName();
            mutation_map = JsonDump(F, I, OpCode, mutation_map);
            std::ofstream o ("genesis_info.json", std::ofstream::trunc);
            o << std::setw(4) << mutation_map << std::endl;
          }
        }
      }
      return false;
    }
  };
}

char SkeletonPass::ID = 0;
static RegisterPass<SkeletonPass> X("Initialization", "Scanning out all the possible mutation points", false, false);

