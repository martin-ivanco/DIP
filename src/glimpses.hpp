#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "logger.hpp"
#include "renderer.hpp"
#include "videoinfo.hpp"

using namespace std;

class Glimpses {

private:
    Logger *log;
    Renderer *renderer;
    VideoInfo video;
    filesystem::path folder;
    vector<VideoInfo> splits;
    vector<VideoInfo> glimpses;

public:
    static const int SPLIT_LENGTH = 5; // seconds
    static const vector<int> PHIS;
    static const vector<int> LAMBDAS;
    static const int WIDTH = 640;
    static const int HEIGHT = 360;

    Glimpses(VideoInfo &video, Renderer &renderer, Logger &log);
    bool render(bool skip_existing = false);
    int length();
    VideoInfo get(int index);
    int splitCount();
    string videoName();

};

#endif // __GLIMPSES__