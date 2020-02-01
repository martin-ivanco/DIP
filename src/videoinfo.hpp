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
    int length; // frames
    int fps;
    int width;
    int height;
    cv::Size size;
    int split = 0;
    double phi = 0;
    double lambda = 0;
    double aov = 0;

    VideoInfo(string path, int split = 0, double phi = 0, double lambda = 0, double aov = 0);
    VideoInfo(string path, int length, int fps, const cv::Size &size, int split = 0, double phi = 0, double lambda = 0, double aov = 0);
};

#endif // __VIDEOINFO__