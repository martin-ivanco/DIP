#ifndef __AUTOCROP__
#define __AUTOCROP__

#include <iostream>
#include <opencv2/core.hpp>

#include "logger.hpp"
#include "saliency.hpp"
#include "trajectory.hpp"
#include "videoinfo.hpp"
#include "../external/saliency/SalMapItti.h"
#include "../external/saliency/SalMapMargolin.h"
#include "../external/saliency/SalMapStentiford.h"
#include "../external/crop/AutocropFang.h"
#include "../external/crop/AutocropStentiford.h"
#include "../external/crop/AutocropSuh.h"

using namespace std;

class AutoCrop {

private:
    static const string FANG_MODEL_PATH;
    static const int STEP_RATIO = 100;
    static const int RATIO_WIDTH = 16;
    static const int RATIO_HEIGHT = 9;
    static constexpr float STENTIFORD_MAX_ZOOM_FACTOR = 1.5f;
    static constexpr float FANG_THRESHOLD = 0.6f;

    Logger *log;

    cv::Mat getSaliencyMap(cv::Mat &frame, int method);
    cv::Rect getROI(cv::Mat &frame, cv::Mat &saliency, int method);
    tPoint getCoords(cv::Mat &frame, cv::Rect &roi);

public:
    static const int FANG = 2;
    static const int STENTIFORD = 0;
    static const int SUH = 1;

    AutoCrop(Logger &log);
    bool findTrajectory(Trajectory &trajectory, VideoInfo input, int method, int step = 1);

};

#endif // __AUTOCROP__