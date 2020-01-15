#include "glimpses.hpp"

using namespace std;
namespace fs = std::filesystem;

const vector<int> Glimpses::PHIS = {-75,-45,-30, -20, -10, 0, 10, 20, 30, 45, 75};
const vector<int> Glimpses::LAMBDAS = {-180, -160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 120, 140, 160};

Glimpses::Glimpses(VideoInfo &video, Renderer &renderer, Logger &log) : video(video) {
    this->renderer = &renderer;
    this->log = &log;
    this->folder = fs::path("data") / fs::path("glimpses") / fs::path(video.name);
    fs::create_directories(this->folder);
}

bool Glimpses::render(bool skip_existing) {
    this->log->info("Rendering glimpses.");
    if (skip_existing)
        this->log->warning("Skipping existing glimpses.");

    // Generating splits
    this->splits = this->renderer->splitVideo(this->video, this->folder, Glimpses::SPLIT_LENGTH,
                                              skip_existing);

    // Generating glimpses - looping over phis and lambdas and for each combination rendering
    // that view from all splits
    cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);
    vector<VideoInfo> views;
    for (auto p : Glimpses::PHIS) {
        for (auto l : Glimpses::LAMBDAS) {
            views = this->renderer->composeViews(this->splits, this->folder, p, l, size,
                                                 skip_existing);
            this->glimpses.insert(this->glimpses.end(), views.begin(), views.end());
        }
    }

    return true;
}

int Glimpses::length() {
    return this->glimpses.size();
}

VideoInfo Glimpses::get(int index) {
    return this->glimpses.at(index);
}

int Glimpses::splitCount() {
    return this->splits.size();
}

string Glimpses::videoName() {
    return this->video.name;
}
