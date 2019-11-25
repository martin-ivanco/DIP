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
#include "constants.hpp"

using namespace std;

class Glimpses {

private:
    VideoInfo originalVideo;
    string folderPath;
    vector<VideoInfo> splits;
    vector<VideoInfo> glimpses;

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
    int length();
    VideoInfo get(int index);
    int splitCount();

    Glimpses(); // for development only
    
};

#endif // __GLIMPSES__