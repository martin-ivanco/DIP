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
    VideoInfo g("", 0, 0, cv::Size(0, 0));
    double score = 0;
    for (int i = 0; i < this->glimpses->length(); i++) {
        g = this->glimpses->get(i);
        score = this->getGlimpseScore(g, method, saveSaliency);
        this->log->debug("Glimpse " + to_string(i) + " of " + to_string(this->glimpses->length())
                         + " scored: " + to_string(score) + ".");
        space.set(g.split, g.phi, g.lambda, g.aov, score);
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
    vector<float> values(Glimpses::HEIGHT * Glimpses::WIDTH * video.get(cv::CAP_PROP_FRAME_COUNT));
    int offset = 0;
    while (true) {
        video.read(frame);
        if (frame.empty())
            break;

        saliency = this->getSaliencyMap(frame, method);
        if ((saliency.rows != Glimpses::HEIGHT) || (saliency.cols != Glimpses::WIDTH)) {
            this->log->error("Unexpected size of saliency map " + to_string(saliency.rows) + "x"
                             + to_string(saliency.cols) + ".");
            return 0;
        }

        // Adding each value to the list
        for (int i = 0; i < Glimpses::HEIGHT; i++) {
            for (int j = 0; j < Glimpses::WIDTH; j++)
                values[offset + i * Glimpses::WIDTH + j] = saliency.at<float>(i, j);
        }

        offset += Glimpses::HEIGHT * Glimpses::WIDTH;
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
        try {
            SalMapMargolin margolin(frame, true);
            return margolin.salMap;
        }
        catch(...) {
            this->log->warning("Exception in getting saliency map by Margolin!");
            return cv::Mat(Glimpses::HEIGHT, Glimpses::WIDTH, CV_32FC1);
        }
    }

    if (method == Saliency::STENTIFORD) {
        SalMapStentiford stentiford(frame);
        stentiford.generateSalMap(DEFAULT_M, DEFAULT_EPS, DEFAULT_T, DEFAULT_THRESHOLD, true);
        return stentiford.salMap;
    }

    this->log->error("Unexpected state in method Saliency::getSaliencyMap()!");
    return cv::Mat();
}
