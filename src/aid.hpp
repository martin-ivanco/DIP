#ifndef __AID__
#define __AID__

#include <iostream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

#include "logger.hpp"
#include "trajectory.hpp"
#include "videoinfo.hpp"
#include "../external/saliency/MBS.hpp"
#include "LandmarkCoreIncludes.h"

using namespace std;

class AID {

private:
    static constexpr double RECALCULATE_INTERVAL = 0.3;
    static const int MAX_TRACKING_POINTS = 100;
    static constexpr double MIN_POINT_QUALITY = 0.01;
    static constexpr double MIN_POINT_DISTANCE = 10;
    static const int MBS_SIZE = 300;
    static const string FACE_DETECTOR_PATH;

    Logger *log;
    LandmarkDetector::FaceDetectorMTCNN face_detector;

    vector<cv::Point2f> getImportantPoints(cv::Mat &frame);
    cv::Mat getSaliencyMap(cv::Mat &frame, bool compact = false);
    cv::Mat getOpticalFlow(cv::Mat &prev_frame, cv::Mat &curr_frame,
                           vector<cv::Point2f> &prev_points, vector<cv::Point2f> &curr_points,
                           bool compact = false);
    cv::Mat getFaceDetection(cv::Mat &frame, bool compact = false);
    bool checkNewShot(vector<cv::Point2f> &prev_points, vector<cv::Point2f> &curr_points, cv::Size frame_size);
    tPoint getCoords(cv::Mat &saliency_map, cv::Mat &optical_flow, cv::Mat &face_detection);
    cv::Rect2f getAreaRect(cv::Point2f &point, cv::Mat &mat, float radius = 5);

public:
    AID(Logger &log);
    bool findTrajectory(Trajectory &trajectory, VideoInfo input, bool continuous = false);

};

#endif // __AID__