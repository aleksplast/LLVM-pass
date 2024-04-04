#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace Pass {

const std::vector<std::string> Colors = {"\"darkgreen\"", "\"blue\"", "\"red\"", "\"orange\"", "\"purple\""};
const size_t NumColors = Colors.size();

const std::string Legend = "subgraph cluster_01 {\n"
    "label = \"Legend\";\n"
    "node [shape=point]\n"
    "{\n"
    "rank=same\n"
    "d0 [style = invis];\n"
    "d1 [style = invis];\n"
    "p0 [style = invis];\n"
    "p1 [style = invis];\n"
    "}\n"
    "d0 -> d1 [label=\"control flow\" style=\"solid\"]\n"
    "p0 -> p1 [label=\"data flow\" style=\"dashed\"]\n"
"}\n";

class UsesInfo {
  size_t BBUsesId;
  size_t InstrUsesId;

public:
  UsesInfo(size_t BBId, size_t InstrId) : BBUsesId(BBId), InstrUsesId(InstrId) {}

  size_t getBBId() {
    return BBUsesId;
  }

  size_t getInstrId() {
    return InstrUsesId;
  }
};

class BinOpInfo {
  int ResultSum = 0;
  size_t OpNum = 0;

public:
  void addResult(int InResult) {
    ResultSum += InResult;
    OpNum += 1;
  }

  double getAverage() {
    if (OpNum == 0) {
      return 0;
    }
    return static_cast<double>(ResultSum) / static_cast<double>(OpNum);
  }
};

class InstructionInfo {
private:
  std::string InstrStr;
  size_t InstrId;
  std::vector<UsesInfo> Uses;
  bool IsBinary = false;
  BinOpInfo Info;

public:
  InstructionInfo(std::string Str, size_t Id, std::vector<UsesInfo> &UsesIn) : InstrStr(Str), InstrId(Id), Uses(UsesIn){};

  void addResult(int InResult) {
    IsBinary = true;
    Info.addResult(InResult);
  }

  void dumpDot(std::ofstream &DotFile, std::string &InnerColor) {
    DotFile << "<TR><TD PORT=\"" << InstrId << "\" COLSPAN=\"2\" BORDER=\"1\" SIDES=\"LR\" BGCOLOR=" << InnerColor << ">";
    DotFile << InstrStr;
    if (IsBinary) {
        DotFile << " = " << Info.getAverage();
    }
    DotFile << "</TD></TR>";
  }

  void dumpUses(std::ofstream &DotFile, size_t BBId, int &EdgeColorId) {
    for (auto &Info: Uses) {
      DotFile << "box" << BBId << ":" << InstrId << ":e";
      DotFile << "->";
      DotFile << "box" << Info.getBBId() << ":" << Info.getInstrId() << ":e";
      DotFile << "[penwidth= \"1\" style = \"dashed\" color=" << Colors[EdgeColorId % NumColors] << "]\n";
      EdgeColorId += 1;
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

  void addInstruction(size_t &InstrId, std::string &InstrStr, std::vector<UsesInfo> &Uses) {
    InstructionsOrder.push_back(InstrId);
    InstrInfo.insert({InstrId, InstructionInfo(InstrStr, InstrId, Uses)});
  }

  void addBinOpResult(size_t &InstrAddr, int &Result) {
    InstrInfo.at(InstrAddr).addResult(Result);
  }

  void addExec() {
    ExecNum += 1;
  }

  void dumpDot(std::ofstream &DotFile, double AvgExecs, int &EdgeColorId) {
    DotFile << "box" << BBId << "[";
    std::string BottomColor, InnerColor, BranchColor;

    if (static_cast<double>(ExecNum) >= AvgExecs) {
        InnerColor = "\"#f9d62e\"";
        BottomColor = "\"#fc913a\"";
        BranchColor = "\"#e2f4c7\"";
    } else {
        InnerColor = "\"#71c7ec\"";
        BottomColor = "\"#189ad3\"";
        BranchColor = "\"#1ebbd7\"";
    }

    DotFile << "label = <";
    DotFile << "<TABLE CELLBORDER=\"1\" CELLSPACING=\"0\" ";
    if (Successors.size() <= 1) {
      DotFile << "BORDER=\"1\" SIDES=\"B\">\n";
    } else {
      DotFile << "BORDER=\"0\">\n";
    }

    DotFile << "<TR><TD PORT = \"f0\" COLSPAN=\"2\" BGCOLOR=" << BottomColor << ">";
    DotFile << "BasicBlock " << BBId << "<BR/>in " << FuncName << "<BR/>Exec num: " << ExecNum << "</TD></TR>";
    for (auto &Id: InstructionsOrder) {
        InstrInfo.at(Id).dumpDot(DotFile, InnerColor);
    }
    if (Successors.size() > 1) {
      DotFile << "<TR ><TD PORT=\"f2\" BGCOLOR=" << BranchColor << ">";
      DotFile << "True</TD><TD PORT=\"f3\" BGCOLOR=" << BranchColor << ">";
      DotFile << "False</TD></TR>";
    }

    DotFile << "</TABLE>>];\n";

    if (Successors.size() == 1) {
      DotFile << "box" << BBId;
      DotFile << "->";
      DotFile << "box" << Successors.front();
      DotFile << "[style = \"solid\"]\n";
    } else if (Successors.size() == 2) {
      DotFile << "box" << BBId << ":f2";
      DotFile << "->" << "box" << Successors.front() << "[style = \"solid\"]\n";
      DotFile << "box" << BBId << ":f3";
      DotFile << "->" << "box" << Successors.back() << "[style = \"solid\"]\n";
    }

    for (auto &Id: InstructionsOrder) {
      InstrInfo.at(Id).dumpUses(DotFile, BBId, EdgeColorId);
    }

  }
};

class CFGVisualizer {
private:
  std::unordered_map<size_t, BBInfo> BasicBlocks;
  double AvgExecs;
  int EdgeColorId = 0;

public:
  CFGVisualizer() = default;

  void handleStaticInfo(std::string StaticFileName) {
    std::ifstream In(StaticFileName, std::ifstream::in);

    while (In.peek() != EOF) {
      size_t BbId;
      std::string FuncName;
      size_t NumInstr;
      size_t NumSucc;

      In >> std::hex >> BbId;
      In >> std::dec >> NumInstr;
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
        size_t UsesId, BBUsesId;
        size_t NumUses;
        std::vector<UsesInfo> Uses;
        std::string InstrStr;

        In >> std::hex >> InstrId >> std::dec >> NumUses;
        for (size_t i = 0; i < NumUses; i++) {
          In >> std::hex >> BBUsesId >> UsesId;
          UsesInfo Info = UsesInfo(BBUsesId, UsesId);
          Uses.push_back(Info);
        }
        In.get();
        std::getline(In, InstrStr);
        BasicBlocks.at(BbId).addInstruction(InstrId, InstrStr, Uses);
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
        size_t InstrId, BBUsesId;
        int Result;
        In >> std::hex >> BbId;
        In >> std::hex >> InstrId;
        In >> std::dec >> Result;

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

    DotFile << "DiGraph CFG {\n";
    // DotFile << "rankdir = TB\n";
    DotFile << "node[shape=plaintext]\n";
    DotFile << Legend;
    for (auto &Bb : BasicBlocks) {
        Bb.second.dumpDot(DotFile, AvgExecs, EdgeColorId);
    }
    DotFile << "}\n";
  }
}; // class CFGVisualizer

} // namespace Pass
