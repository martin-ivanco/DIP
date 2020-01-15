#ifndef __RENDERER__
#define __RENDERER__

#include <iostream>
#include <cmath>
#include <filesystem>
#include <string>
#include <tuple>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "logger.hpp"
#include "videoinfo.hpp"

using namespace std;

class Renderer { // TODO Refactor

private:
    static const int FOURCC;

    Logger *log;

    string getSplitName(int timeBlock);
    string getViewName(int timeBlock, int longitude, int latitude);
    tuple<cv::Mat, cv::Mat> getStereographicDisplacementMaps(cv::Size &sourceSize, cv::Size &projectionSize, double phi, double lambda, double aov = 104.3);
    double deg2rad(double deg);
    tuple<double, double> rad2erp(cv::Size planeSize, double phi, double lambda);
    bool remapFrame(cv::VideoCapture &capture, cv::VideoWriter &writer, cv::Mat &map1, cv::Mat &map2, cv::Mat &frame, cv::Mat &warped);
    void open(cv::VideoCapture &capture, string filename);
    void open(cv::VideoWriter &writer, string filename, int fps, cv::Size &size);
    void close(cv::VideoCapture &capture, cv::VideoWriter &writer);

public:
    Renderer(Logger &log);
    vector<VideoInfo> splitVideo(VideoInfo &video, string outputFolder, int splitLength, bool skipExisting = false);
    vector<VideoInfo> composeViews(vector<VideoInfo> &videos, string outputFolder, int phi, int lambda, cv::Size &viewSize, bool skipExisting = false);
    VideoInfo renderTrajectory(VideoInfo &video, vector<tuple<double, double, double>> &trajectory, VideoInfo &output);

};

#endif // __RENDERER__