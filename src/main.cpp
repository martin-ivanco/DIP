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
    Logger log(true);
    ArgParse arg(argc, argv, log);
    if (! arg.parse()) 
        return 1;

    vector<string> input_paths;
    if (! arg.getInputs(input_paths))
        return 1;

    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);
    for (auto path : input_paths) {
        log.debug("Processing input path '" + path + "'.");
        Renderer renderer(path);

        if (arg.method == ArgParse::AUTOCROP) {
            log.debug("Using automatic cropping method.");
            AutoCrop autoCrop(log);
            vector<tuple<double, double, double>> trajectory;

            if (arg.submethod == ArgParse::AUTOCROP_SUH) {
                if (! autoCrop.findTrajectory(trajectory, path, AutoCrop::SUH))
                    return 3;
            }
            if (arg.submethod == ArgParse::AUTOCROP_STE) {
                if (! autoCrop.findTrajectory(trajectory, path, AutoCrop::STENTIFORD))
                    return 3;
            }
            if (arg.submethod == ArgParse::AUTOCROP_FAN) {
                if (! autoCrop.findTrajectory(trajectory, path, AutoCrop::FANG))
                    return 3;
            }

            if (arg.submethod == ArgParse::AUTOCROP_360) {
                log.error("Sorry, the 360 saliency method is not yet supported.");
                return 2;
            }

            renderer.renderPath(trajectory, size);
        }

        if (arg.method == ArgParse::GLIMPSES) {
            log.debug("Using spatio-temporal glimpses method.");
            Glimpses glimpses(renderer, log, arg.skip == ArgParse::SKIP_GLIMPSES);
            vector<tuple<int, int>> trajectory;

            if (arg.submethod == ArgParse::GLIMPSES_C3D) {
                log.warning("This method is incomplete. C3D will be generated.");
                C3D c3d(glimpses, log);
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
