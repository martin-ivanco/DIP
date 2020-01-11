#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>

#include "logger.hpp"
#include "renderer.hpp"
#include "videoinfo.hpp"

using namespace std;

class Glimpses {

private:
    Logger *log;
    Renderer *renderer;
    vector<VideoInfo> splits;
    vector<VideoInfo> glimpses;

    void fillQuick(); // for development only

public:
    static const int SPLIT_LENGTH = 5; // seconds
    static const vector<int> PHIS;
    static const vector<int> LAMBDAS;
    static const int WIDTH = 640;
    static const int HEIGHT = 360;

    Glimpses(Renderer &renderer, Logger &log, bool skip_existing = false);
    int length();
    VideoInfo get(int index);
    VideoInfo getOriginalVideo();
    int splitCount();

};

#endif // __GLIMPSES__