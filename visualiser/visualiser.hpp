#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace Pass {

class InstructionInfo {
private:
  std::string InstrStr;
  size_t InstrId;
  bool IsBinary = false;
  size_t Result = 0;

public:
  InstructionInfo(std::string Str, size_t Id) : InstrStr(Str), InstrId(Id){};

  void setResult(int InResult) {
    IsBinary = true;
    Result = InResult;
  }

  void dumpDot(std::ofstream &DotFile) {
    DotFile << InstrStr;
    if (IsBinary) {
        DotFile << " = " << Result;
    }
  }
};

class BBInfo {
  std::string FuncName;
  size_t BBId;
  size_t ExecNum = 0;
  std::vector<size_t> InstructionsOrder;
  std::unordered_map<size_t, InstructionInfo> InstrInfo;
  std::vector<size_t> Successors;

public:
  BBInfo(std::string Name, size_t Id) : FuncName(Name), BBId(Id){};

  void addSuccessor(size_t &SuccessorId) { Successors.push_back(SuccessorId); }

  void addInstruction(size_t &InstrId, std::string &InstrStr) {
    InstructionsOrder.push_back(InstrId);
    InstrInfo.insert({InstrId, InstructionInfo(InstrStr, InstrId)});
  }

  void addBinOpResult(size_t &InstrAddr, int &Result) {
    InstrInfo.at(InstrAddr).setResult(Result);
  }

  void addExec() {
    ExecNum += 1;
  }

  void dumpDot(std::ofstream &DotFile, double AvgExecs) {
    DotFile << "\"box" << BBId << "\" ";
    DotFile << "[shape = \"record\", style = \"filled\", gradientangle = 90, fillcolor = \"";

    if (static_cast<double>(ExecNum) >= AvgExecs) {
        DotFile << "gold;" << 1.0 - 3.0 / (static_cast<double>(InstructionsOrder.size()) + 3.0) << ":orange";
    } else {
        DotFile << "cyan;" << 1.0 - 3.0 / (static_cast<double>(InstructionsOrder.size()) + 3.0) << ":deepskyblue";
    }

    DotFile << "\", label = \"";
    DotFile << "{BasicBlock\\n" << BBId << " in " << FuncName << "\\nExec num: " << ExecNum << "\\n";
    for (auto &Id: InstructionsOrder) {
        InstrInfo.at(Id).dumpDot(DotFile);
        DotFile << "\\n";
    }
    DotFile << "}\"]\n";

    for (auto &Succ: Successors) {
        DotFile << "\"box" << BBId << "\"";
        DotFile << " -> ";
        DotFile << "\"box" << Succ << "\"";
        DotFile << "[style = \"solid\"]";
    }
  }
};

class CFGVisualiser {
private:
  std::unordered_map<size_t, BBInfo> BasicBlocks;
  double AvgExecs;

public:
  CFGVisualiser() = default;

  void handleStaticInfo(std::string StaticFileName) {
    std::ifstream In(StaticFileName, std::ifstream::in);

    while (In.peek() != EOF) {
      size_t BbId;
      std::string FuncName;
      size_t NumInstr;
      size_t NumSucc;

      In >> std::hex >> BbId;
      In >> NumInstr;
      In >> FuncName;
      In >> NumSucc;

      BasicBlocks.insert({BbId, BBInfo(FuncName, BbId)});

      for (int i = 0; i < NumSucc; i++) {
        size_t Successor;

        In >> std::hex >> Successor;
        BasicBlocks.at(BbId).addSuccessor(Successor);
      }

      char ch = In.get();
      ch = In.get();
      if (ch != '{') {
        std::cerr << "Parsing gone wrong\n";
        return;
      }

      for (int i = 0; i < NumInstr; i++) {
        size_t InstrId;
        std::string InstrStr;

        In >> std::hex >> InstrId;
        In.get();
        std::getline(In, InstrStr);
        BasicBlocks.at(BbId).addInstruction(InstrId, InstrStr);
      }

      ch = In.get();
      if (ch != '}') {
        std::cerr << "Parsing gone wrong\n";
        return;
      }
      In.get();
      In.get();
    }
  }

  void handleDynamicInfo(std::string DynamicFileName) {
    std::ifstream In(DynamicFileName);
    size_t TotalExecs;

    while (In.peek() != EOF) {
      std::string Type;
      size_t BbId;

      In >> Type;
      if (Type == "binop") {
        size_t InstrId;
        int Result;
        In >> std::hex >> BbId;
        In >> std::hex >> InstrId;
        In >> Result;

        BasicBlocks.at(BbId).addBinOpResult(InstrId, Result);
      } else if (Type == "exec") {
        In >> std::hex >> BbId;
        TotalExecs += 1;

        BasicBlocks.at(BbId).addExec();
      }

      In.get();
    }

    AvgExecs = static_cast<double>(TotalExecs) / static_cast<double>(BasicBlocks.size());
  }

  void dumpDot(std::string DotFileName) {
    std::ofstream DotFile(DotFileName);

    DotFile << "DiGraph List {\n";
    DotFile << "rankdir = TB\n";
    for (auto &Bb : BasicBlocks) {
        Bb.second.dumpDot(DotFile, AvgExecs);
    }
    DotFile << "}\n";
  }
}; // class CFGVisualiser

} // namespace Pass
