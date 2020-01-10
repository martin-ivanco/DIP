#include "saliency.hpp"

using namespace std;

Saliency::Saliency(Glimpses &glimpses, int saliencyType) 
    : glimpses(glimpses), space(glimpses.splitCount()) {
    // this->space.loadFromFile(); // for development only - skip salinecy
    // return;

    if (saliencyType == Saliency::ITTI)
        this->getSaliencyMap = this->getSaliencyMapItti;
    if (saliencyType == Saliency::MARGOLIN)
        this->getSaliencyMap = this->getSaliencyMapMargolin;
    if (saliencyType == Saliency::STENTIFORD)
        this->getSaliencyMap = this->getSaliencyMapStentiford;

    VideoInfo g = VideoInfo("", 0, cv::Size(0, 0));
    for (int i = 0; i < glimpses.length(); i++) {
        g = this->glimpses.get(i);
        this->space.set(g.split, g.phi, g.lambda, this->getGlimpseScoreQuick(g)); // for development only 
    }
}

ScoreSpace Saliency::getScoreSpace() {
    return this->space;
}

double Saliency::getGlimpseScore(VideoInfo &glimpse) {
    // TODO better median
    cv::VideoCapture video = cv::VideoCapture(glimpse.path);
    if (! video.isOpened())
        return 0;

    vector<double> scores;
    cv::Mat frame;
    while (true) {
        video.read(frame);
        if (frame.empty())
            break;
        scores.push_back(this->getFrameScore(frame));
    }
    sort(scores.begin(), scores.end());
    double result = scores[scores.size() / 2];

    return result;
}

double Saliency::getFrameScore(cv::Mat &frame) {
    // TODO median
    cv::Mat saliencyMap = this->getSaliencyMap(frame);
    double sum = 0;
    for (int i = 0; i < Glimpses::HEIGHT; i++) {
        for (int j = 0; j < Glimpses::WIDTH; j++)
            sum += saliencyMap.at<uchar>(i, j);
    }
    return sum / (Glimpses::WIDTH * Glimpses::HEIGHT);
}

cv::Mat Saliency::getSaliencyMapItti(cv::Mat &frame) {
    SalMapItti itti = SalMapItti(frame);
    return itti.salMap;
}

cv::Mat Saliency::getSaliencyMapMargolin(cv::Mat &frame) {
    SalMapMargolin margolin = SalMapMargolin(frame);
    return margolin.salMap;
}

cv::Mat Saliency::getSaliencyMapStentiford(cv::Mat &frame) {
    SalMapStentiford stentiford = SalMapStentiford(frame);
    stentiford.generateSalMap();
    return stentiford.salMap;
}

// for development only
double Saliency::getGlimpseScoreQuick(VideoInfo &glimpse) {
    cv::VideoCapture video = cv::VideoCapture(glimpse.path);
    if (! video.isOpened())
        return 0;

    double scoreSum = 0;
    int counter = 0;
    cv::Mat frame;

    for (int i = 0; i < 2 * glimpse.fps; i++) {
        video.read(frame);
    }

    video.read(frame);
    if (frame.empty())
        return 0;

    return this->getFrameScore(frame);
}
