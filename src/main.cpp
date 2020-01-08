#include <iostream>
#include <string>

#include "autocrop.hpp"
#include "renderer.hpp"
#include "glimpses.hpp"
#include "saliency.hpp"
#include "scorespace.hpp"
#include "tools.hpp"
#include "c3d.hpp"

using namespace std;
namespace fs = std::filesystem;

static const string INPUT_PATH = "data/input";

int main(int argc, char **argv) {
    // TODO CHECK INPUT PATH N STUFF
    try {
        Renderer renderer = Renderer("data/input/test.mp4");
        // cerr << "Creating autocrop." << endl;
        // AutoCrop autocrop = AutoCrop("../Playground/test.mp4", S_ITTI);
        // cerr << "Rendering video." << endl;
        // renderer.renderPath(autocrop.getPath());
        Glimpses glimpses = Glimpses(renderer);
        C3D c3d = C3D(glimpses);
        // Saliency saliency = Saliency(glimpses, S_ITTI);
        // ScoreSpace space = saliency.getScoreSpace();
        // vector<tuple<int, int>> path = space.getBestPath();
        // for (auto i: path)
        //     Tools::print(to_string(get<0>(i)) + " " + to_string(get<0>(i)) + "\n", L_DEBUG);
        // renderer.renderSplitPath(path);
    }
    catch (const invalid_argument& a) {
        cerr << "Invalid argument. The file doesn't exist or isn't an mp4." << endl;
        return 1;
    }
    catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 2;
    }
    
    return 0;
}