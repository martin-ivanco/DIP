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

bool autocrop(VideoInfo input, ArgParse &arg, Trajectory &trajectory, Logger &log) {
    log.info("Using automatic cropping method.");

    // Detremining automatic cropping method to use
    int method = -1;
    if (arg.submethod == ArgParse::AUTOCROP_FAN) method = AutoCrop::FANG;
    if (arg.submethod == ArgParse::AUTOCROP_STE) method = AutoCrop::STENTIFORD;
    if (arg.submethod == ArgParse::AUTOCROP_SUH) method = AutoCrop::SUH;
    if (arg.submethod == ArgParse::AUTOCROP_360) {
        log.error("Sorry, the 360 saliency method is not yet supported.");
        return false;
    }

    // Evaluating trajectory using chosen method and smoothing it if requested
    AutoCrop autoCrop(log);
    if (! autoCrop.findTrajectory(trajectory, input, method))
        return false;
    if (arg.smooth)
        trajectory.smooth(Glimpses::SPLIT_LENGTH * input.fps);
    
    return true;
}

bool evaluateGlimpses(Glimpses &glimpses, ScoreSpace &space, ArgParse &arg, Logger &log) {
    if (arg.submethod == ArgParse::GLIMPSES_C3D) {
        // Extracting C3D features
        C3D c3d(glimpses, log);
        c3d.prepare();
        c3d.extract();

        // Evaluating using chosen category
        return c3d.evaluate(space, arg.category);
    }
    else {
        // Detremining saliency mapping method to use
        int method = -1;
        if (arg.submethod == ArgParse::GLIMPSES_ITT) method = Saliency::ITTI;
        if (arg.submethod == ArgParse::GLIMPSES_MAR) method = Saliency::MARGOLIN;
        if (arg.submethod == ArgParse::GLIMPSES_STE) method = Saliency::STENTIFORD;

        // Evaluating saliency
        Saliency saliency(glimpses, log);
        return saliency.evaluate(space, method);
    }
}

bool autocam(Glimpses &glimpses, ArgParse &arg, Trajectory &trajectory, Logger &log) {
    log.info("Using spatio-temporal glimpses method.");

    // Coarse search
    if (! glimpses.renderCoarse(arg.skip & ArgParse::SKIP_GLIMPSES))
        return false;
    ScoreSpace space(glimpses.splitCount(), log);
    if (! evaluateGlimpses(glimpses, space, arg, log))
        return false;
    if (! space.findTrajectory(trajectory, glimpses.splitLength() * 2, Glimpses::ANGLE_EPS * 2))
        return false;
    // for development only
    space.save(fs::path("data") / fs::path("output") / fs::path("coarse_space.txt"));
    trajectory.save(fs::path("data") / fs::path("output") / fs::path("coarse_trajectory.txt"));

    // Dense search
    glimpses.clear();
    if (! glimpses.renderDense(trajectory, arg.skip & ArgParse::SKIP_GLIMPSES))
        return false;
    space = ScoreSpace(glimpses.splitCount(), log);
    if (! evaluateGlimpses(glimpses, space, arg, log))
        return false;
    if (! space.findTrajectory(trajectory, glimpses.splitLength(), Glimpses::ANGLE_EPS))
        return false;
    // for development only
    space.save(fs::path("data") / fs::path("output") / fs::path("dense_space.txt"));
    trajectory.save(fs::path("data") / fs::path("output") / fs::path("dense_trajectory.txt"));

    return true;
}

int main(int argc, char **argv) {
    // Setting up logger and parsing arguments
    Logger log(true);
    ArgParse arg(argc, argv, log);
    if (! arg.parse()) 
        return 1;

    // Finding inputs
    vector<string> input_paths;
    if (! arg.getInputs(input_paths))
        return 1;

    // Creating output folder and renderer
    fs::path outputFolder = fs::path("data") / fs::path("output");
    fs::create_directories(outputFolder);
    Renderer renderer(log);
    
    // Processing input videos
    for (auto path : input_paths) {
        log.info("Processing input path '" + path + "'.");
        VideoInfo input(path);
        fs::path outputPath(outputFolder / fs::path(path).filename());
        VideoInfo output(outputPath.string(), 0, static_cast<double>(input.fps), input.size);
        Trajectory trajectory(log, input.length);

        // Using standard automatic cropping
        if (arg.method == ArgParse::AUTOCROP) {
            if (! autocrop(input, arg, trajectory, log))
                return 2;
        }

        // Using spatio-temporal glimpses
        if (arg.method == ArgParse::GLIMPSES) {
            Glimpses glimpses(input, renderer, log);
            if (! autocam(glimpses, arg, trajectory, log))
                return 2;
        }

        // Preparing dataset files - generating glimpses and extracting C3D features
        if (arg.method == ArgParse::DATASET) {
            log.info("Preparing dataset files.");
            Glimpses glimpses(input, renderer, log);
            glimpses.renderAll(1, arg.submethod == ArgParse::DATASET_2D);
            C3D c3d(glimpses, log);
            c3d.prepare(arg.submethod == ArgParse::DATASET_2D);
            c3d.extract(arg.submethod == ArgParse::DATASET_2D);
            c3d.save();
            continue;
        }

        renderer.renderTrajectory(input, trajectory, output);
    }

    return 0;
}
