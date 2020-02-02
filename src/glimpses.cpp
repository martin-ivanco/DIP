#include "glimpses.hpp"

using namespace std;
namespace fs = std::filesystem;

const vector<int> Glimpses::PHIS = {-75,-45,-30, -20, -10, 0, 10, 20, 30, 45, 75};
const vector<int> Glimpses::LAMBDAS = {-160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60,
                                       80, 100, 120, 140, 160, 180};
const vector<double> Glimpses::AOVS = {46.4, 65.5, 104.3};

Glimpses::Glimpses(VideoInfo &video, Renderer &renderer, Logger &log) : video(video) {
    this->renderer = &renderer;
    this->log = &log;
    this->folder = fs::path("data") / fs::path("glimpses") / fs::path(video.name);
    fs::create_directories(this->folder);
}

bool Glimpses::renderCoarse(bool skip_existing) {
    this->log->info("Rendering coarsely sampled glimpses.");
    if (skip_existing)
        this->log->warning("Skipping existing glimpses.");

    // Generating splits
    this->splits = this->renderer->splitVideo(this->video, this->folder, Glimpses::SPLIT_LENGTH * 2,
                                              skip_existing);

    // Generating coarsely sampled glimpses - looping over every second phi and lambda and for each
    // combination rendering that view from all splits
    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);
    vector<VideoInfo> views;
    for (int p = 0; p < Glimpses::PHIS.size(); p += 2) {
        for (int l = 0; l < Glimpses::LAMBDAS.size(); l += 2) {
            views = this->renderer->composeViews(this->splits, this->folder, Glimpses::PHIS[p],
                                                 Glimpses::LAMBDAS[l], Glimpses::AOVS[2], size,
                                                 skip_existing);
            this->glimpses.insert(this->glimpses.end(), views.begin(), views.end());
        }
    }

    return true;
}

bool Glimpses::renderDense(Trajectory &trajectory, bool skip_existing) {
    this->log->info("Rendering densely sampled glimpses.");
    if (skip_existing)
        this->log->warning("Skipping existing glimpses.");

    // Generating splits
    this->splits = this->renderer->splitVideo(this->video, this->folder, Glimpses::SPLIT_LENGTH,
                                              skip_existing);

    // Generating densely sampled glimpses - going through the selected trajectory and rendering
    // glimpses close to it
    int splitLength = Glimpses::SPLIT_LENGTH * this->video.fps;
    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);
    vector<VideoInfo> inputs;
    vector<VideoInfo> views;
    for (int i = splitLength / 2; i < trajectory.length(); i += splitLength) {
        for (auto p : Glimpses::PHIS) {
            if (abs(trajectory[i].phi - p) > Glimpses::ANGLE_EPS)
                continue;
            for (auto l : Glimpses::LAMBDAS) {
                double diff = abs(fmod(trajectory[i].lambda - l + 180, 360)) - 180;
                if (abs(diff) > Glimpses::ANGLE_EPS)
                    continue;
                for (auto a : Glimpses::AOVS) {
                    inputs = {this->splits[i / splitLength]};
                    views = this->renderer->composeViews(inputs, this->folder, p, l, a, size,
                                                         skip_existing);
                    this->glimpses.insert(this->glimpses.end(), views.begin(), views.end());
                }
            }
        }
    }

    // Doing one more iteration if the trajectory length is not a multiple of split length
    if (trajectory.length() % splitLength != 0) {
        int endIndex = trajectory.length() - (trajectory.length() % splitLength);
        endIndex += trajectory.length() % splitLength == 0
                    ? splitLength / 2 : (trajectory.length() % splitLength) / 2;
        for (auto p : Glimpses::PHIS) {
            if (abs(trajectory[endIndex].phi - p) > Glimpses::ANGLE_EPS)
                continue;
            for (auto l : Glimpses::LAMBDAS) {
                double diff = abs(fmod(trajectory[endIndex].lambda - l + 180, 360)) - 180;
                if (abs(diff) > Glimpses::ANGLE_EPS)
                    continue;
                for (auto a : Glimpses::AOVS) {
                    inputs = {this->splits[this->splits.size() - 1]};
                    views = this->renderer->composeViews(inputs, this->folder, p, l, a, size,
                                                         skip_existing);
                    this->glimpses.insert(this->glimpses.end(), views.begin(), views.end());
                }
            }
        }
    }

    return true;
}

bool Glimpses::renderAll(int aov, bool splits_only) {
    if (aov == -1) 
        this->log->info("Rendering all glimpses at all angles of view.");
    else if ((aov >= 0) && (aov < Glimpses::AOVS.size()))
        this->log->info("Rendering all glimpses at angle of view "
                        + to_string(Glimpses::AOVS[aov]) + "°.");
    else {
        this->log->error("Unavailable angle of view index (" + to_string(aov) + ").");
        return false;
    }

    // Generating splits
    this->splits = this->renderer->splitVideo(this->video, this->folder, Glimpses::SPLIT_LENGTH);
    if (splits_only)
        return true;

    // Generating all glimpses at all directions and selected angles of view
    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);
    vector<VideoInfo> views;
    for (int p = 0; p < Glimpses::PHIS.size(); p++) {
        for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
            if (aov == -1) {
                for (int a = 0; a < Glimpses::AOVS.size(); a++) {
                    views = this->renderer->composeViews(this->splits, this->folder,
                                                         Glimpses::PHIS[p], Glimpses::LAMBDAS[l],
                                                         Glimpses::AOVS[a], size);
                }   
            }
            else {
                views = this->renderer->composeViews(this->splits, this->folder,
                                                     Glimpses::PHIS[p], Glimpses::LAMBDAS[l],
                                                     Glimpses::AOVS[aov], size);
            }
            this->glimpses.insert(this->glimpses.end(), views.begin(), views.end());
        }
    }

    return true;
}

int Glimpses::length() {
    return this->glimpses.size();
}

VideoInfo Glimpses::get(int index, bool get_split) {
    if (get_split)
        return this->splits[index];
    return this->glimpses[index];
}

int Glimpses::splitCount() {
    return this->splits.size();
}

string Glimpses::videoName() {
    return this->video.name;
}

void Glimpses::clear() {
    this->splits.clear();
    this->glimpses.clear();
}
