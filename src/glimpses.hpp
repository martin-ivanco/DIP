#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>

#include "videoinfo.hpp"
#include "renderer.hpp"
#include "constants.hpp"

using namespace std;

class Glimpses {

private:
    Renderer renderer;
    vector<VideoInfo> splits;
    vector<VideoInfo> glimpses;

    void fillQuick(); // for development only

public:
    Glimpses(Renderer renderer);
    int length();
    VideoInfo get(int index);
    int splitCount();

};

#endif // __GLIMPSES__