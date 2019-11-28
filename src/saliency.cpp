#include "saliency.hpp"

using namespace std;

Saliency::Saliency(Glimpses glimpses, int saliencyType) : glimpses(glimpses), space(glimpses.splitCount()) {
    if (SKIP_SALIENCY) {
        this->space.loadFromFile();
        return;
    }

    if (saliencyType == S_ITTI)
        this->getSaliencyMap = this->getSaliencyMapItti;
    if (saliencyType == S_MARGOLIN)
        this->getSaliencyMap = this->getSaliencyMapMargolin;
    if (saliencyType == S_STENTIFORD)
        this->getSaliencyMap = this->getSaliencyMapStentiford;

    VideoInfo g = VideoInfo("", 0, cv::Size(0, 0));
    for (int i = 0; i < glimpses.length(); i++) {
        g = this->glimpses.get(i);
        Tools::print("Evaluating glimpse " + to_string(i + 1) + " of " + to_string(glimpses.length()) + "\n", L_DEBUG);
        // this->space.set(g.split, g.phi, g.lambda, this->getGlimpseScore(g));
        this->space.set(g.split, g.phi, g.lambda, this->getGlimpseScoreQuick(g)); // for development only 
    }
}

ScoreSpace Saliency::getScoreSpace() {
    return this->space;
}

double Saliency::getGlimpseScore(VideoInfo glimpse) {
    cv::VideoCapture video = cv::VideoCapture(glimpse.path);
    if (! video.isOpened())
        return 0;

    double scoreSum = 0;
    int counter = 0;
    cv::Mat frame;
    while (true) {
        if (counter % glimpse.fps == 0)
            Tools::print("=", L_DEBUG);
        video.read(frame);
        if (frame.empty())
            break;
        counter++;
        scoreSum += this->getFrameScore(frame);
    }

    Tools::print("\nScore: " + to_string(counter != 0 ? scoreSum / counter : 0) + "\n", L_DEBUG);
    return counter != 0 ? scoreSum / counter : 0;
}

double Saliency::getFrameScore(cv::Mat frame) {
    cv::Mat saliencyMap = this->getSaliencyMap(frame);
    double sum = 0;
    for (int i = 0; i < GLIMPSE_HEIGHT; i++) {
        for (int j = 0; j < GLIMPSE_WIDTH; j++)
            sum += saliencyMap.at<uchar>(i, j);
    }
    return sum / (GLIMPSE_WIDTH * GLIMPSE_HEIGHT);
}

cv::Mat Saliency::getSaliencyMapItti(cv::Mat frame) {
    SalMapItti itti = SalMapItti(frame);
    return itti.salMap;
}

cv::Mat Saliency::getSaliencyMapMargolin(cv::Mat frame) {
    SalMapMargolin margolin = SalMapMargolin(frame);
    return margolin.salMap;
}

cv::Mat Saliency::getSaliencyMapStentiford(cv::Mat frame) {
    SalMapStentiford stentiford = SalMapStentiford(frame);
    stentiford.generateSalMap();
    return stentiford.salMap;
}

// for development only
double Saliency::getGlimpseScoreQuick(VideoInfo glimpse) {
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
