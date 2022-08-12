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
#include <algorithm>
#include <array>
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
        int prev_node_length = 0;
        auto prev_node = df_begin(&CG);
        json path_array = json::array();

        for(auto bnode = CG.begin(), enode = CG.end(); bnode!=enode; bnode++) {
            if(bnode->second != nullptr)
            {
                CallGraphNode *cgn = bnode->second.get();
                if(Function *fptr = cgn->getFunction()) {
                    std::vector<std::string> path_node;

                    json single_path = json::object();
                    errs() << "fptr" << fptr->getName() << "\n";
                    dbgs() << "operator" << cgn->size()<< "\n";
                    if (cgn->size()>0) {
                        for(int i=0; i < cgn->size(); i++) {
                            if(cgn->operator[](i) != nullptr)
                            {
                                if(cgn->operator[](i)->getFunction() != nullptr) {

                                    if(!(std::count(std::begin(path_node), std::end(path_node), cgn->operator[](i)->getFunction()->getName()) > 0)) {
                                        path_node.push_back(cgn->operator[](i)->getFunction()->getName());
                                    }
                                }
                            }
                        }
                        single_path[fptr->getName()] = path_node;
                        path_array.push_back(single_path);
                    }

                }
            }
        }



        std::ofstream o ("callgraphnode_callgraph.json", std::ofstream::trunc);
        o << std::setw(4) << path_array << std::endl;

    }
};
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("CallGraph", "Generate callgraph", false, false);
