#include "glimpses.hpp"

using namespace std;
namespace fs = std::filesystem;

const vector<int> Glimpses::PHIS = {-75,-45,-30, -20, -10, 0, 10, 20, 30, 45, 75};
const vector<int> Glimpses::LAMBDAS = {-180, -160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 120, 140, 160};

Glimpses::Glimpses(Renderer &renderer, Logger &log, bool skip_existing) {
    this->log = &log;
    this->renderer = &renderer;
    if (skip_existing)
        this->log->debug("Skipping existing glimpses.");

    this->splits = renderer.splitVideo(Glimpses::SPLIT_LENGTH, skip_existing);
    
    vector<VideoInfo> views;
    for (auto p : Glimpses::PHIS) {
        for (auto l : Glimpses::LAMBDAS) {
            cv::Size size(Glimpses::WIDTH, Glimpses::HEIGHT);
            views = renderer.composeViews(p, l, this->splits, size, skip_existing);
            this->glimpses.insert(this->glimpses.end(), views.begin(), views.end());
        }
    }
}

int Glimpses::length() {
    return this->glimpses.size();
}

VideoInfo Glimpses::get(int index) {
    return this->glimpses.at(index);
}

VideoInfo Glimpses::getOriginalVideo() {
    return this->renderer->getVideoInfo();
}

int Glimpses::splitCount() {
    return this->splits.size();
}

// for development only - deprecated
void Glimpses::fillQuick() {
    VideoInfo originalVideo = this->renderer->getVideoInfo();
    fs::path p = fs::path(originalVideo.folder);
    string folderPath = p / originalVideo.name;

    char buffer[100];
    for (int i = 0; i < 6; i++) {
        sprintf (buffer, "%s_g%.4d.mp4", originalVideo.name.c_str(), static_cast<int>(this->splits.size()));
        string splitPath = fs::path(folderPath) / string(buffer);
        VideoInfo split = VideoInfo(splitPath, static_cast<double>(originalVideo.fps), originalVideo.size);
        this->splits.push_back(split);
    }
    
    for (auto p : PHIS) {
        for (auto l : LAMBDAS) {
            for (int i = 0; i < this->splits.size(); i++) {
                sprintf (buffer, "%s_g%.4d_h%.3d_v%.3d.mp4", originalVideo.name.c_str(), i, l, p);
                string glimpsePath = fs::path(folderPath) / string(buffer);
                VideoInfo glimpse = VideoInfo(glimpsePath, static_cast<double>(originalVideo.fps), cv::Size(Glimpses::WIDTH, Glimpses::HEIGHT), i, p, l);
                this->glimpses.push_back(glimpse);
            }
        }
    }
}