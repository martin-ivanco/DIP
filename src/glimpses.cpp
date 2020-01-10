#include "glimpses.hpp"

using namespace std;
namespace fs = std::filesystem;

Glimpses::Glimpses(Renderer renderer) : renderer(renderer) {
    // this->fillQuick(); // for development only - skip glimpses
    // return;

    this->splits = renderer.splitVideo(Glimpses::SPLIT_LENGTH);
    
    vector<VideoInfo> views;
    for (auto p : Glimpses::PHIS) {
        for (auto l : Glimpses::LAMBDAS) {
            views = renderer.composeViews(p, l, this->splits);
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
    return this->renderer.getVideoInfo();
}

int Glimpses::splitCount() {
    return this->splits.size();
}

// for development only
void Glimpses::fillQuick() {
    VideoInfo originalVideo = this->renderer.getVideoInfo();
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