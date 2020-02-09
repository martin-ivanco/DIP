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

            // Coarse search
            glimpses.renderCoarse(arg.skip & ArgParse::SKIP_GLIMPSES);
            ScoreSpace space(glimpses.splitCount(), log);

            // Using C3D features
            if (arg.submethod == ArgParse::GLIMPSES_C3D) {
                // Extracting C3D features
                C3D c3d(glimpses, log);
                c3d.prepare();
                c3d.extract();

                // Evaluating using chosen category
                if (! c3d.evaluate(space, arg.category))
                if (! space.findTrajectory(trajectory, Glimpses::SPLIT_LENGTH * 2 * input.fps,
                                           Glimpses::ANGLE_EPS * 2))
                    return 3;
                space.save(fs::path("data") / fs::path("output") / fs::path("coarse_space.txt")); // for development only
                trajectory.save(fs::path("data") / fs::path("output") / fs::path("coarse_trajectory.txt")); // for development only
            }
            // Using saliency mapping
            else {
                // Initializing saliency and score space
                Saliency saliency(glimpses, log);

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
                if (arg.skip & ArgParse::SKIP_SALIENCY) {
                    space.load(fs::path("data") / fs::path("output") / fs::path("coarse_space.txt")); // for development only
                }
                else {
                    if (! saliency.evaluate(space, method))
                        return 3;
                    space.save(fs::path("data") / fs::path("output") / fs::path("coarse_space.txt")); // for development only
                }
                if (! space.findTrajectory(trajectory, Glimpses::SPLIT_LENGTH * 2 * input.fps,
                                           Glimpses::ANGLE_EPS * 2))
                    return 3;
                space.save(fs::path("data") / fs::path("output") / fs::path("coarse_space.txt")); // for development only
                trajectory.save(fs::path("data") / fs::path("output") / fs::path("coarse_trajectory.txt")); // for development only
            }
                
            // Dense search
            glimpses.clear();
            glimpses.renderDense(trajectory, arg.skip & ArgParse::SKIP_GLIMPSES);
            space = ScoreSpace(glimpses.splitCount(), log);

            // Using C3D features
            if (arg.submethod == ArgParse::GLIMPSES_C3D) {
                log.warning("This method is incomplete. C3D will be generated.");
                C3D c3d(glimpses, log);
                c3d.prepare();
                c3d.extract();
                return 2;
            }
            // Using saliency mapping
            else {
                // Initializing saliency and score space
                Saliency saliency(glimpses, log);

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
                space.save(fs::path("data") / fs::path("output") / fs::path("dense_space.txt")); // for development only
                if (! space.findTrajectory(trajectory, Glimpses::SPLIT_LENGTH * input.fps,
                                           Glimpses::ANGLE_EPS))
                    return 3;
                space.save(fs::path("data") / fs::path("output") / fs::path("dense_space.txt")); // for development only
                trajectory.save(fs::path("data") / fs::path("output") / fs::path("dense_trajectory.txt")); // for development only
            }
        }

        // Preparing dataset files - generating glimpses and extracting C3D features
        if (arg.method == ArgParse::DATASET) {
            log.info("Preparing dataset files.");
            Glimpses glimpses(input, renderer, log);
            glimpses.renderAll(1, arg.submethod == ArgParse::DATASET_2D);
            C3D c3d(glimpses, log);
            c3d.prepare(arg.submethod == ArgParse::DATASET_2D);
            c3d.extract();
            c3d.save();
            continue;
        }

        renderer.renderTrajectory(input, trajectory, output);
    }

    return 0;
}
