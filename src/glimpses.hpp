#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>

#include "videoinfo.hpp"
#include "renderer.hpp"

using namespace std;

class Glimpses {

private:
    Renderer renderer;
    vector<VideoInfo> splits;
    vector<VideoInfo> glimpses;

    void fillQuick(); // for development only

public:
    static const int SPLIT_LENGTH = 5; // seconds
    static const int PHIS[11] = {-75,-45,-30, -20, -10, 0, 10, 20, 30, 45, 75};
    static const int LAMBDAS[18] = {-180, -160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 120, 140, 160};
    static const int WIDTH = 640;
    static const int HEIGHT = 360;

    Glimpses(Renderer renderer);
    int length();
    VideoInfo get(int index);
    VideoInfo getOriginalVideo();
    int splitCount();

};

#endif // __GLIMPSES__