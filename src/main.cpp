#include <iostream>
#include <vector>
#include <string>
#include <tuple>

#include "argparse.hpp"
#include "autocrop.hpp"
#include "c3d.hpp"
#include "glimpses.hpp"
#include "renderer.hpp"
#include "saliency.hpp"
#include "scorespace.hpp"

using namespace std;

static const string INPUT_PATH = "data/input";

int main(int argc, char **argv) {
    ArgParse arg(argc, argv);
    string err = arg.parse();
    if (! err.empty()) {
        cerr << "ERROR: " << err << endl;
        return 1;
    }

    vector<string> input_paths;
    err = arg.getInputs(input_paths);
    if (! err.empty()) {
        cerr << "ERROR: " << err << endl;
        return 1;
    }

    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);

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
            cerr << "ERROR: Sorry, the 360 saliency method is not yet supported." << endl;
            return 2;
        }

        renderer.renderPath(path, size);
    }

    if (arg.method == ArgParse::GLIMPSES) {
        Renderer renderer("data/input/test.mp4");
        Glimpses glimpses(renderer);
        vector<tuple<int, int>> path;

        if (arg.submethod == ArgParse::GLIMPSES_C3D) {
            cerr << "WARNING: This method is incomplete. C3D will be generated." << endl;
            C3D c3d(glimpses);
        }

        if (arg.submethod == ArgParse::GLIMPSES_ITT) {
            Saliency sal(glimpses, Saliency::ITTI);
            path = sal.getScoreSpace().getBestPath();
        }
        if (arg.submethod == ArgParse::GLIMPSES_STE) {
            Saliency sal(glimpses, Saliency::STENTIFORD);
            path = sal.getScoreSpace().getBestPath();
        }
        if (arg.submethod == ArgParse::GLIMPSES_MAR) {
            Saliency sal(glimpses, Saliency::MARGOLIN);
            path = sal.getScoreSpace().getBestPath();
        }

        renderer.renderSplitPath(path, Glimpses::PHIS, Glimpses::LAMBDAS, size, Glimpses::SPLIT_LENGTH);
    }

    return 0;
}
