#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <tuple>

#include "argparse.hpp"
#include "renderer.hpp"
#include "autocrop.hpp"
#include "glimpses.hpp"
#include "c3d.hpp"
#include "saliency.hpp"
#include "scorespace.hpp"

using namespace std;
namespace fs = std::filesystem;

static const string INPUT_PATH = "data/input";

int main(int argc, char **argv) {
    ArgParse arg(argc, argv);
    string err = arg.parse();
    if (! err.empty()) {
        cerr << "ERROR: " << err << endl;
        return 1;
    }
        
    // TODO CHECK INPUT PATH
    
    if (arg.method == ArgParse::AUTOCROP) {
        Renderer renderer("data/input/test.mp4");
        vector<tuple<double, double, double>> path;

        if (arg.submethod == ArgParse::AUTOCROP_SUH)
            path = AutoCrop("data/input/test.mp4", Saliency::ITTI).getPath();
        if (arg.submethod == ArgParse::AUTOCROP_STE)
            path = AutoCrop("data/input/test.mp4", Saliency::STENTIFORD).getPath();
        if (arg.submethod == ArgParse::AUTOCROP_FAN)
            path = AutoCrop("data/input/test.mp4", Saliency::MARGOLIN).getPath();

        if (arg.submethod == ArgParse::AUTOCROP_360) {
            cerr << "ERROR: " << "Sorry, the 360 saliency method is not yet supported." << endl;
            return 1;
        }

        renderer.renderPath(path);
    }

    if (arg.method == ArgParse::GLIMPSES) {
        Renderer renderer("data/input/test.mp4");
        vector<tuple<int, int>> path;

        if (arg.submethod == ArgParse::GLIMPSES_C3D) {
            cerr << "WARNING: " << "This method is incomplete. C3D will be generated." << endl;
            Glimpses glimpses(renderer);
            C3D c3d(glimpses);
        }

        if (arg.submethod == ArgParse::GLIMPSES_ITT) {
            Saliency sal(Glimpses(renderer), Saliency::ITTI);
            path = sal.getScoreSpace().getBestPath();
        }
        if (arg.submethod == ArgParse::GLIMPSES_STE) {
            Saliency sal(Glimpses(renderer), Saliency::STENTIFORD);
            path = sal.getScoreSpace().getBestPath();
        }
        if (arg.submethod == ArgParse::GLIMPSES_MAR) {
            Saliency sal(Glimpses(renderer), Saliency::MARGOLIN);
            path = sal.getScoreSpace().getBestPath();
        }

        renderer.renderSplitPath(path);
    }

    // try {
    //     Renderer renderer = Renderer("data/input/test.mp4");
    //     // cerr << "Creating autocrop." << endl;
    //     // AutoCrop autocrop = AutoCrop("../Playground/test.mp4", S_ITTI);
    //     // cerr << "Rendering video." << endl;
    //     // renderer.renderPath(autocrop.getPath());
    //     Glimpses glimpses = Glimpses(renderer);
    //     C3D c3d = C3D(glimpses);
    //     // Saliency saliency = Saliency(glimpses, S_ITTI);
    //     // ScoreSpace space = saliency.getScoreSpace();
    //     // vector<tuple<int, int>> path = space.getBestPath();
    //     // for (auto i: path)
    //     //     Tools::print(to_string(get<0>(i)) + " " + to_string(get<0>(i)) + "\n", L_DEBUG);
    //     // renderer.renderSplitPath(path);
    // }
    // catch (const invalid_argument& a) {
    //     cerr << "Invalid argument. The file doesn't exist or isn't an mp4." << endl;
    //     return 1;
    // }
    // catch (const runtime_error& e) {
    //     cerr << e.what() << endl;
    //     return 2;
    // }
    
    return 0;
}