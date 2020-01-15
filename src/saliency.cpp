#include "saliency.hpp"

using namespace std;

Saliency::Saliency(Glimpses &glimpses, Logger &log) {
    this->glimpses = &glimpses;
    this->log = &log;
}

bool Saliency::evaluate(ScoreSpace &space, int method, bool saveSaliency) {
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

    // Looping over glimpses and evaluating each one
    VideoInfo g("", 0, cv::Size(0, 0));
    double score = 0;
    for (int i = 0; i < this->glimpses->length(); i++) {
        g = this->glimpses->get(i);
        score = this->getGlimpseScore(g, method, saveSaliency);
        space.set(g.split, g.phi, g.lambda, score);
        this->log->debug("Glimpse " + to_string(i) + " of " + to_string(this->glimpses->length())
                         + " scored: " + to_string(score) + ".");
    }

    return true;
}

double Saliency::getGlimpseScore(VideoInfo &glimpse, int method, bool saveSaliency) {
    // Opening video
    cv::VideoCapture video(glimpse.path);
    if (! video.isOpened()) {
        this->log->error("Couldn't open glimpse '" + glimpse.path + "' for saliency evaluation.");
        return 0;
    }
    
    // Getting saliency map of each frame
    cv::Mat frame;
    cv::Mat saliency;
    vector<float> values;
    while (true) {
        video.read(frame);
        if (frame.empty())
            break;

        saliency = this->getSaliencyMap(frame, method);

        // Adding each value to the list
        for (int i = 0; i < Glimpses::HEIGHT; i++) {
            for (int j = 0; j < Glimpses::WIDTH; j++)
                values.push_back(saliency.at<float>(i, j));
        }
    }

    // Finding median
    sort(values.begin(), values.end());
    return values[values.size() / 2];
}

cv::Mat Saliency::getSaliencyMap(cv::Mat &frame, int method) {
    if (method == Saliency::ITTI) {
        SalMapItti itti(frame, true);
        return itti.salMap;
    }

    if (method == Saliency::MARGOLIN) {
        SalMapMargolin margolin(frame, true);
        return margolin.salMap;
    }

    if (method == Saliency::STENTIFORD) {
        SalMapStentiford stentiford(frame);
        stentiford.generateSalMap(DEFAULT_M, DEFAULT_EPS, DEFAULT_T, DEFAULT_THRESHOLD, true);
        return stentiford.salMap;
    }

    this->log->error("Unexpected state in method Saliency::getSaliencyMap()!");
    return cv::Mat();
}
