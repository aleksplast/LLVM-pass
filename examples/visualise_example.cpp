#include "../visualiser/visualiser.hpp"

using namespace Pass;

int main() {
    CFGVisualiser Visualiser;

    Visualiser.handleStaticInfo("./run_example.pcno");
    Visualiser.handleDynamicInfo("./run_example.pcda");
    Visualiser.dumpDot("cfg.dot");

    return 0;
}
