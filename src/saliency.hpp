#ifndef __SALIENCY__
#define __SALIENCY__

#include <iostream>
#include <string>

#include "logger.hpp"
#include "glimpses.hpp"
#include "scorespace.hpp"
#include "videoinfo.hpp"
#include "../external/saliency/SalMapItti.h"
#include "../external/saliency/SalMapMargolin.h"
#include "../external/saliency/SalMapStentiford.h"

using namespace std;

class Saliency {

private:
    Logger *log;
    Glimpses *glimpses;

    // TODO refactor
    double getGlimpseScore(VideoInfo &glimpse, int method);
    double getFrameScore(cv::Mat &frame, int method);
    cv::Mat getSaliencyMap(cv::Mat &frame, int method);

public:
    static const int ITTI = 0;
    static const int MARGOLIN = 1;
    static const int STENTIFORD = 2;

    Saliency(Glimpses &glimpses, Logger &log);
    bool evaluate(ScoreSpace &space, int method);

};

#endif // __SALIENCY__