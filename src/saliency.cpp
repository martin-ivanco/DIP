#include "saliency.hpp"

using namespace std;

Saliency::Saliency(Glimpses &glimpses, Logger &log) {
    this->glimpses = &glimpses;
    this->log = &log;
}

bool Saliency::evaluate(ScoreSpace &space, int method) {
    // Determininig the method to use
    if (method == Saliency::ITTI)
        this->log->info("Evaluating saliency of glimpses using method by Itti.");
    else if (method == Saliency::MARGOLIN)
        this->log->info("Evaluating saliency of glimpses using method by Margolin.");
    else if (method == Saliency::STENTIFORD)
        this->log->info("Evaluating saliency of glimpses using method by Stentiford.");
    else {
        this->log->error("Unknown saliency method.");
        return false;
    }

    VideoInfo g("", 0, cv::Size(0, 0));
    for (int i = 0; i < this->glimpses->length(); i++) {
        g = this->glimpses->get(i);
        space.set(g.split, g.phi, g.lambda, this->getGlimpseScore(g, method));
    }

    return true;
}

double Saliency::getGlimpseScore(VideoInfo &glimpse, int method) {
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
        scores.push_back(this->getFrameScore(frame, method));
    }
    sort(scores.begin(), scores.end());
    double result = scores[scores.size() / 2];

    return result;
}

double Saliency::getFrameScore(cv::Mat &frame, int method) {
    // TODO median
    cv::Mat saliencyMap = this->getSaliencyMap(frame, method);
    double sum = 0;
    for (int i = 0; i < Glimpses::HEIGHT; i++) {
        for (int j = 0; j < Glimpses::WIDTH; j++)
            sum += saliencyMap.at<uchar>(i, j);
    }
    return sum / (Glimpses::WIDTH * Glimpses::HEIGHT);
}

cv::Mat Saliency::getSaliencyMap(cv::Mat &frame, int method) {
    if (method == Saliency::ITTI) {
        SalMapItti itti(frame);
        return itti.salMap;
    }

    if (method == Saliency::MARGOLIN) {
        SalMapMargolin margolin(frame);
        return margolin.salMap;
    }

    if (method == Saliency::STENTIFORD) {
        SalMapStentiford stentiford(frame);
        stentiford.generateSalMap();
        return stentiford.salMap;
    }

    this->log->error("Unexpected state in method Saliency::getSaliencyMap()!");
    return cv::Mat();
}
