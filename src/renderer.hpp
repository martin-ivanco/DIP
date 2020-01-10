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

using namespace std;

class Renderer { // TODO Refactor

private:
    static const int FOURCC;
    static const cv::Size GLIMPSE_SIZE;

    VideoInfo originalVideo;
    string folderPath;

    string getSplitName(int timeBlock);
    string getViewName(int timeBlock, int longitude, int latitude);
    tuple<cv::Mat, cv::Mat> getStereographicDisplacementMaps(double phi, double lambda, cv::Size &size, double aov = 104.3);
    double deg2rad(double deg);
    tuple<double, double> rad2erp(double phi, double lambda);
    bool remapFrame(cv::VideoCapture &capture, cv::VideoWriter &writer, cv::Mat &map1, cv::Mat &map2, cv::Mat &frame, cv::Mat &warped);
    void open(cv::VideoCapture &capture, string filename);
    void open(cv::VideoWriter &writer, string filename, int fps, cv::Size &size);
    void close(cv::VideoCapture &capture, cv::VideoWriter &writer);

public:
    Renderer(string videoFilePath);
    vector<VideoInfo> splitVideo(int splitLength);
    vector<VideoInfo> composeViews(int phi, int lambda, vector<VideoInfo> &videos, cv::Size &size);
    VideoInfo renderSplitPath(vector<tuple<int, int>> &path, const vector<int> &phis, const vector<int> &lambdas, cv::Size &size, int split_length);
    VideoInfo renderPath(vector<tuple<double, double, double>> &path, cv::Size &size);
    
    VideoInfo getVideoInfo(); // for development only
};

#endif // __RENDERER__