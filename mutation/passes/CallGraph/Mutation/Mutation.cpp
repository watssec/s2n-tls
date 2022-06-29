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
#include "llvm/Analysis/CallGraph.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include "string"
#include "cstring"

using namespace llvm;
using json = nlohmann::json;

static cl::opt<std::string> InputFileName("file_name", cl::desc("Specify input filename"), cl::value_desc("filename"));
static cl::opt<int> FunctionId("function_num", cl::desc("Specify function number"), cl::value_desc("function number"));
static cl::opt<int> InstructionId("instruction_num", cl::desc("Specify instruction number"), cl::value_desc("instruction number"));
// In a DFS, if the Pathlength of the current node is smaller or equal to the previous one, then it has reached a leaf node.
// Once a leaf node is found, dump the path.

namespace {


  struct SkeletonPass : public ModulePass {
    static char ID;


    SkeletonPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {

    CallGraph CG = CallGraph(M); 
    int cnt = 0;
    int prev_node_length = 1;
    auto prev_node = df_begin(&CG);
    json path_array = json::array();
    
    for (auto IT = df_begin(&CG), EI = df_end(&CG); IT != EI; IT++) {
  
     
      

      
      if (prev_node_length > IT.getPathLength()){
        
        std::vector<std::string> path_node;  
       
        json single_path = json::object();
        
        for(int i = 0; i< prev_node_length; i++){
          dbgs() << "prev" << prev_node.getPath(i)->getFunction() << "\n";
          
          if(Function *F = prev_node.getPath(i)->getFunction()){
            path_node.push_back(F->getName());
            dbgs() << "fun name " << F->getName() << "\n";
          }else{
            path_node.push_back("0x0");
            dbgs() << "no name" << "0x0" << "\n";
          }
          
        }
        
        single_path[std::to_string(cnt)] = path_node; 
        path_array.push_back(single_path);
        
      }
      
      prev_node_length = IT.getPathLength();
      
      prev_node++;
      cnt = cnt +1;
    }

    std::ofstream o ("call_graph_all.json", std::ofstream::trunc);
    o << std::setw(4) << path_array << std::endl;
  
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("CallGraph", "Generate callgraph", false, false);
