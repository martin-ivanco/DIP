#ifndef __VIDEOINFO__
#define __VIDEOINFO__

#include <iostream>
#include <filesystem>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

using namespace std;

struct VideoInfo {
    string path;
    string folder;
    string name;
    int fps;
    int width;
    int height;
    cv::Size size;

    VideoInfo(string path);
    VideoInfo(string path, int fps, cv::Size size);
};

#endif // __VIDEOINFO__