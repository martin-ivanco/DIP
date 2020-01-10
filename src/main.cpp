#include <iostream>
#include <vector>
#include <string>
#include <tuple>

#include "argparse.hpp"
#include "autocrop.hpp"
#include "c3d.hpp"
#include "glimpses.hpp"
#include "logger.hpp"
#include "renderer.hpp"
#include "saliency.hpp"
#include "scorespace.hpp"

using namespace std;

int main(int argc, char **argv) {
    ArgParse arg(argc, argv);
    string err = arg.parse();
    if (! err.empty()) {
        cerr << "ERROR: " << err << endl;
        return 1;
    }

    Logger log(arg.verbose, true);
    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);

    vector<string> input_paths;
    err = arg.getInputs(input_paths);
    if (! err.empty()) {
        log.error(err);
        return 1;
    }

    for (auto path : input_paths) {
        log.debug("Processing input path '" + path + "'.");
        Renderer renderer(path);

        if (arg.method == ArgParse::AUTOCROP) {
            log.debug("Using automatic cropping method.");
            vector<tuple<double, double, double>> trajectory;

            if (arg.submethod == ArgParse::AUTOCROP_SUH)
                trajectory = AutoCrop(path, Saliency::ITTI).getPath();
            if (arg.submethod == ArgParse::AUTOCROP_STE)
                trajectory = AutoCrop(path, Saliency::STENTIFORD).getPath();
            if (arg.submethod == ArgParse::AUTOCROP_FAN)
                trajectory = AutoCrop(path, Saliency::MARGOLIN).getPath();

            if (arg.submethod == ArgParse::AUTOCROP_360) {
                log.error("Sorry, the 360 saliency method is not yet supported.");
                return 2;
            }

            renderer.renderPath(trajectory, size);
        }

        if (arg.method == ArgParse::GLIMPSES) {
            log.debug("Using spatio-temporal glimpses method.");
            Glimpses glimpses(renderer);
            vector<tuple<int, int>> trajectory;

            if (arg.submethod == ArgParse::GLIMPSES_C3D) {
                log.warning("This method is incomplete. C3D will be generated.");
                C3D c3d(glimpses);
            }

            if (arg.submethod == ArgParse::GLIMPSES_ITT) {
                Saliency sal(glimpses, Saliency::ITTI);
                trajectory = sal.getScoreSpace().getBestPath();
            }
            if (arg.submethod == ArgParse::GLIMPSES_STE) {
                Saliency sal(glimpses, Saliency::STENTIFORD);
                trajectory = sal.getScoreSpace().getBestPath();
            }
            if (arg.submethod == ArgParse::GLIMPSES_MAR) {
                Saliency sal(glimpses, Saliency::MARGOLIN);
                trajectory = sal.getScoreSpace().getBestPath();
            }

            renderer.renderSplitPath(trajectory, Glimpses::PHIS, Glimpses::LAMBDAS, size, Glimpses::SPLIT_LENGTH);
        }
    }

    return 0;
}
