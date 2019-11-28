#ifndef __SALIENCY__
#define __SALIENCY__

#include <iostream>
#include <string>

#include "glimpses.hpp"
#include "constants.hpp"
#include "scorespace.hpp"
#include "tools.hpp"
#include "../external/saliency/SalMapItti.h"
#include "../external/saliency/SalMapMargolin.h"
#include "../external/saliency/SalMapStentiford.h"

using namespace std;

class Saliency {

private:
    Glimpses glimpses;
    cv::Mat (* getSaliencyMap)(cv::Mat);
    ScoreSpace space;

    double getGlimpseScoreQuick(VideoInfo glimpse);  // for development only
    double getGlimpseScore(VideoInfo glimpse);
    double getFrameScore(cv::Mat frame);

    static cv::Mat getSaliencyMapItti(cv::Mat frame);
    static cv::Mat getSaliencyMapMargolin(cv::Mat frame);
    static cv::Mat getSaliencyMapStentiford(cv::Mat frame);

public:
    Saliency(Glimpses glimpses, int saliencyType);
    ScoreSpace getScoreSpace();

};

#endif // __SALIENCY__