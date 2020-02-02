#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cmath>

#include "logger.hpp"
#include "renderer.hpp"
#include "trajectory.hpp"
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
    static const vector<double> AOVS;
    static const int ANGLE_EPS = 30; // degrees
    static const int WIDTH = 640;
    static const int HEIGHT = 360;

    Glimpses(VideoInfo &video, Renderer &renderer, Logger &log);
    bool renderCoarse(bool skip_existing = false);
    bool renderDense(Trajectory &trajectory, bool skip_existing = false);
    bool renderAll(int aov = -1, bool splits_only = false);
    int length();
    VideoInfo get(int index, bool get_split = false);
    int splitCount();
    string videoName();
    void clear();

};

#endif // __GLIMPSES__