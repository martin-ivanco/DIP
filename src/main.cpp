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
#include "trajectory.hpp"
#include "videoinfo.hpp"

using namespace std;

int main(int argc, char **argv) {
    Logger log(true);
    ArgParse arg(argc, argv, log);
    if (! arg.parse()) 
        return 1;

    vector<string> input_paths;
    if (! arg.getInputs(input_paths))
        return 1;

    fs::path outputFolder = fs::path("data") / fs::path("output");
    fs::create_directories(outputFolder);
    Renderer renderer(log);
    
    for (auto path : input_paths) {
        log.info("Processing input path '" + path + "'.");
        VideoInfo input(path);
        fs::path outputPath(outputFolder / fs::path(path).filename());
        VideoInfo output(outputPath.string(), 0, static_cast<double>(input.fps), input.size);
        Trajectory trajectory(log, input.length);

        // Using standard automatic cropping
        if (arg.method == ArgParse::AUTOCROP) {
            log.info("Using automatic cropping method.");
            AutoCrop autoCrop(log);

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
        }

        // Using spatio-temporal glimpses
        if (arg.method == ArgParse::GLIMPSES) {
            log.info("Using spatio-temporal glimpses method.");
            Glimpses glimpses(input, renderer, log);
            glimpses.render(arg.skip == ArgParse::SKIP_GLIMPSES);

            // Using C3D features
            if (arg.submethod == ArgParse::GLIMPSES_C3D) {
                log.warning("This method is incomplete. C3D will be generated.");
                C3D c3d(glimpses, log);
                return 2;
            }
            // Using saliency mapping
            else {
                // Initializing saliency and score space
                Saliency saliency(glimpses, log);
                ScoreSpace space(glimpses.splitCount(), Glimpses::SPLIT_LENGTH * input.fps);

                // Detremining saliency mapping method to use
                int method = -1;
                if (arg.submethod == ArgParse::GLIMPSES_ITT) method = Saliency::ITTI;
                if (arg.submethod == ArgParse::GLIMPSES_MAR) method = Saliency::MARGOLIN;
                if (arg.submethod == ArgParse::GLIMPSES_STE) method = Saliency::STENTIFORD;
                if (method == -1) {
                    log.error("Unknown saliency method.");
                    return 3;
                }

                // Evaluating saliency and finding the best trajectory
                if (! saliency.evaluate(space, method))
                    return 3;
                if (! space.findTrajectory(trajectory))
                    return 3;
            }
        }

        renderer.renderTrajectory(input, trajectory, output);
    }

    return 0;
}
