#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "videoinfo.hpp"

using namespace std;

static const int PHIS[11] = {-75,-45,-30, -20, -10, 0, 10, 20, 30, 45, 75};
static const int LAMBDAS[18] = {-180, -160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 120, 140, 160};
static const int GLIMPSE_WIDTH = 640;
static const int GLIMPSE_HEIGHT = 360;

class Glimpses {

private:
    VideoInfo originalVideo;
    string folderPath;
    vector<VideoInfo> splits;

    void generateGlimpses();
    void splitVideo();
    void composeView(int phi, int lambda);
    string getSplitName(int timeBlock);
    string getGlimpseName(int timeBlock, int longitude, int latitude);
    tuple<cv::Mat, cv::Mat> getGnomonicDisplacementMaps(int phi, int lambda);
    tuple<cv::Mat, cv::Mat> getStereographicDisplacementMaps(int phi, int lambda);
    double deg2rad(double deg);
    tuple<double, double> rad2erp(double phi, double lambda);

public:
    Glimpses(string videoFilePath);
    
};

#endif // __GLIMPSES__