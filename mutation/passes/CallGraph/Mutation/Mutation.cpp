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

using namespace llvm;

static cl::opt<std::string> InputFileName("file_name", cl::desc("Specify input filename"), cl::value_desc("filename"));
static cl::opt<int> FunctionId("function_num", cl::desc("Specify function number"), cl::value_desc("function number"));
static cl::opt<int> InstructionId("instruction_num", cl::desc("Specify instruction number"), cl::value_desc("instruction number"));


namespace {


  struct SkeletonPass : public ModulePass {
    static char ID;


    SkeletonPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {

    CallGraph CG = CallGraph(M); 
      
    for (auto IT = df_begin(&CG), EI = df_end(&CG); IT != EI; IT++) {
      dbgs() << "get Path Length" << IT.getPathLength() << "\n";
      if (Function *F = IT->getFunction()) {
        
        dbgs() << "Visiting function: " << F->getName() << "\n";
      }
    }
    }
  };
}

char SkeletonPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static RegisterPass<SkeletonPass> X("CallGraph", "Generate callgraph", false, false);
