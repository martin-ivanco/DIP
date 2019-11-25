#ifndef __RENDERER__
#define __RENDERER__

#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "videoinfo.hpp"
#include "constants.hpp"

using namespace std;

class Renderer {

private:
    VideoInfo originalVideo;
    string folderPath;

    string getSplitName(int timeBlock);
    string getViewName(int timeBlock, int longitude, int latitude);
    tuple<cv::Mat, cv::Mat> getGnomonicDisplacementMaps(int phi, int lambda);
    tuple<cv::Mat, cv::Mat> getStereographicDisplacementMaps(int phi, int lambda);
    double deg2rad(double deg);
    tuple<double, double> rad2erp(double phi, double lambda);

public:
    Renderer(string videoFilePath);
    vector<VideoInfo> splitVideo(int splitLength);
    vector<VideoInfo> composeViews(int phi, int lambda, vector<VideoInfo> videos);
    VideoInfo renderPath(vector<tuple<int, int>> path);
    
    VideoInfo getVideoInfo(); // for development only
};

#endif // __RENDERER__