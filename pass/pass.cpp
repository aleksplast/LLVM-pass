#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <fstream>
#include <llvm-14/llvm/IR/BasicBlock.h>
#include <llvm-14/llvm/IR/CFG.h>
#include <llvm-14/llvm/IR/Function.h>
#include <llvm-14/llvm/IR/Instruction.h>
#include <llvm-14/llvm/IR/LLVMContext.h>
#include <llvm-14/llvm/Support/FileSystem.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <system_error>
#include <unordered_set>

using namespace llvm;
using sys::fs::OpenFlags;

namespace {

struct CFGPass : public FunctionPass {
  static char ID;
  std::error_code Er;
  std::unordered_set<std::string> Files;
  CFGPass() : FunctionPass(ID) {}

  virtual bool runOnFunction(Function &F) {
    if (isLogger(F.getName())) {
      return false;
    }

    std::error_code Er;
    std::string Filename = handleSourceName(F.getParent()->getSourceFileName());
    std::string StaticFile = Filename + "pcno";
    std::string DynamicFile = Filename + "pcda";
    size_t InstrNum = 0;
    bool Changed = false;

    if (Files.find(Filename) == Files.end()) {
      raw_fd_ostream StatOs(StaticFile, Er);
      raw_fd_ostream DynOs(DynamicFile, Er);
      Files.insert(Filename);
    }

    raw_fd_ostream Os(StaticFile, Er, OpenFlags::OF_Append);

    // Dumping static info
    for (auto &Bb : F) {
      Os << &Bb << "\n" << Bb.getInstList().size() << "\n" << F.getName();
      Os << "\n" << Bb.getTerminator()->getNumSuccessors() << "\n";
      for (auto *Succ : successors(&Bb)) {
        Os << Succ << "\n";
      }
      Os << "{\n";
      for (auto &I : Bb) {
        Os << &I << " " << I.getNumUses() <<  "\n";
        for (auto& U: I.uses()) {
            auto* User = U.getUser();
            if (auto *Instr = dyn_cast<Instruction>(User)) {
              Os << Instr->getParent() << " " << User << "\n";
            }
        }
        Os << I << "\n";
      }
      Os << "}\n\n";
    }

    // Prepare builder for IR modification
    LLVMContext &Ctx = F.getContext();
    IRBuilder<> Builder(Ctx);
    Type *RetType = Type::getVoidTy(Ctx);

    // Prepare bbStartLogger function
    ArrayRef<Type *> BbStartLogParamTypes = {
        Builder.getInt8Ty()->getPointerTo(),
        Builder.getInt64Ty()};
    FunctionType *BbStartLogFuncType =
        FunctionType::get(RetType, BbStartLogParamTypes, false);
    FunctionCallee BbStartLogFunc =
        F.getParent()->getOrInsertFunction("bbStartLogger", BbStartLogFuncType);

    // Prepare binOpLogger function
    ArrayRef<Type *> binOpParamTypes = {Builder.getInt8Ty()->getPointerTo(),
                                        Builder.getInt64Ty(),
                                        Builder.getInt64Ty()};
    FunctionType *binOpLogFuncType =
        FunctionType::get(RetType, binOpParamTypes, false);
    FunctionCallee binOpLogFunc =
        F.getParent()->getOrInsertFunction("binOpLogger", binOpLogFuncType);

    // Insert loggers for bbStart and binOpt instructions
    for (auto &Bb : F) {
      Builder.SetInsertPoint(&*Bb.getFirstInsertionPt());
      Value *FileName = Builder.CreateGlobalStringPtr(DynamicFile);
      Value *BbAddr = ConstantInt::get(Builder.getInt64Ty(), (int64_t)(&Bb));
      Value *args[] = {FileName, BbAddr};
      Builder.CreateCall(BbStartLogFunc, args);

      for (auto &I : Bb) {
        if (auto *op = dyn_cast<BinaryOperator>(&I)) {
          // Insert after op
          Builder.SetInsertPoint(op);
          Builder.SetInsertPoint(&Bb, ++Builder.GetInsertPoint());
          Value *v = Builder.CreateIntCast(op, Builder.getInt64Ty(), true);

          // Insert a call to binOptLogFunc function
          Value *InstrAddr =
              ConstantInt::get(Builder.getInt64Ty(), (int64_t)(&I));
          Value *args[] = {FileName, BbAddr, InstrAddr};
          Builder.CreateCall(binOpLogFunc, args);
        }
      }

      Changed = true;
    }

    return Changed;
  }

  std::string handleSourceName(const std::string &SourceFile) {
    size_t SlashPos = SourceFile.find_last_of('/');
    size_t DotPos = SourceFile.find_last_of('.');
    std::string CovFileName;

    if (SlashPos == SourceFile.npos) {
      SlashPos = 0;
      CovFileName = SourceFile.substr(SlashPos, DotPos + 1 - SlashPos);
    } else {
      CovFileName = SourceFile.substr(SlashPos + 1, DotPos - SlashPos);
    }

    return CovFileName;
  }

  bool isLogger(StringRef FuncName) {
    return FuncName == "bbStartLogger" || FuncName == "binOpLogger";
  }

}; // struct CFGPass

} // namespace

char CFGPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerMyPass(const PassManagerBuilder &,
                           legacy::PassManagerBase &PM) {
  PM.add(new CFGPass());
}
static RegisterStandardPasses
    RegisterMyPass(PassManagerBuilder::EP_OptimizerLast, registerMyPass);
