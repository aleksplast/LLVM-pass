#include "../visualizer/visualizer.hpp"

using namespace Pass;

int main(int argc, char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Wrong parameters, try again\n";
    }
    std::string StaticFile(argv[1]);
    std::string DynamicFile(argv[2]);
    std::string OutFile;
    if (argc == 4) {
        OutFile = argv[3];
    } else {
        OutFile = "cfg.dot";
    }

    CFGVisualizer Visualizer;

    Visualizer.handleStaticInfo(StaticFile);
    Visualizer.handleDynamicInfo(DynamicFile);
    Visualizer.dumpDot(OutFile);

    return 0;
}
