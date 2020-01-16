#include "autocrop.hpp"

using namespace std;

const string AutoCrop::FANG_MODEL_PATH = "../external/crop/models/Trained_model21.yml";

AutoCrop::AutoCrop(Logger &log) {
    this->log = &log;
}

bool AutoCrop::findTrajectory(Trajectory &trajectory, string videoFilePath, int method, int step) {
    // Opening input video
    cv::VideoCapture video = cv::VideoCapture(videoFilePath);
    if (! video.isOpened()) {
        this->log->error("Couldn't open video.");
        return false;
    }

    // Determining the method to use
    if (method == AutoCrop::FANG)
        this->log->info("Using method by Fang.");
    else if (method == AutoCrop::STENTIFORD)
        this->log->info("Using method by Stentiford.");
    else if (method == AutoCrop::SUH)
        this->log->info("Using method by Suh.");
    else {
        this->log->error("Unknown AutoCrop method.");
        return false;
    }

    // Temporary variables
    cv::Mat frame;
    video.read(frame);
    cv::Mat saliencyMap;
    cv::Rect roi;

    // Main evaluation loop - get saliency, get crop, get coordinate
    for (int i = 0; ! frame.empty(); i++) {
        this->log->debug("Evaluating frame " + to_string(i) + ".");

        saliencyMap = this->getSaliencyMap(frame, method);
        roi = this->getROI(frame, saliencyMap, method);
        trajectory[i] = this->getCoords(frame, roi);

        // Skip (step - 1) frames
        for (int j = 0; j < step - 1; j++){
            video.read(frame);
            i++;
        }
    }

    return true;
}

cv::Mat AutoCrop::getSaliencyMap(cv::Mat &frame, int method) {
    this->log->debug("Getting saliency map.");

    if (method == AutoCrop::FANG) {
        SalMapMargolin margolin(frame);
        return margolin.salMap;
    }

    if (method == AutoCrop::STENTIFORD) {
        SalMapStentiford stentiford(frame);
        stentiford.generateSalMap();
        return stentiford.salMap;
    }

    if (method == AutoCrop::SUH) {
        SalMapItti itti(frame);
        return itti.salMap;
    }

    this->log->error("Unexpected state in method AutoCrop::getSaliencyMap()!");
    return cv::Mat();
}

cv::Rect AutoCrop::getROI(cv::Mat &frame, cv::Mat &saliency, int method) {
    this->log->debug("Getting cropping rectangle.");

    // Count step variables for cropping algorithms
    int hStep = frame.cols / AutoCrop::STEP_RATIO;
	int vStep = frame.rows / AutoCrop::STEP_RATIO;
	if (hStep < 1) hStep = 1;
	if (vStep < 1) vStep = 1;

    if (method == AutoCrop::FANG) {
        AutocropFang fang(frame, saliency, AutoCrop::FANG_MODEL_PATH);
        fang.WHratioCrop(AutoCrop::RATIO_WIDTH, AutoCrop::RATIO_HEIGHT, hStep, vStep);
        return cv::Rect(fang.getX(), fang.getY(), fang.getWidth(), fang.getHeight());
    }

    if (method == AutoCrop::STENTIFORD) {
        AutocropStentiford stentiford(saliency);
        stentiford.randomWHratio(AutoCrop::RATIO_WIDTH, AutoCrop::RATIO_HEIGHT,
                                 AutoCrop::STENTIFORD_MAX_ZOOM_FACTOR);
        return cv::Rect(stentiford.getX(), stentiford.getY(),
                        stentiford.getWidth(), stentiford.getHeight());
    }

    if (method == AutoCrop::SUH) {
        AutocropSuh suh(saliency);
        suh.bruteForceWHratio(AutoCrop::RATIO_WIDTH, AutoCrop::RATIO_HEIGHT,
                              AutoCrop::FANG_THRESHOLD, hStep, vStep);
        return cv::Rect(suh.getX(), suh.getY(), suh.getWidth(), suh.getHeight());
    }

    this->log->error("Unexpected state in method AutoCrop::getROI()!");
    return cv::Rect();
}

tPoint AutoCrop::getCoords(cv::Mat &frame, cv::Rect &roi) {
    this->log->debug("Getting spherical coordinates.");

    // Count spherical coordinates from center of the cropping window and its width
    double lam = (roi.x + roi.width / 2.0) * (2 * CV_PI) / frame.size().width;
    double phi = (roi.y + roi.height / 2.0)* CV_PI / frame.size().height;
    return tPoint(phi - CV_PI / 2, lam - CV_PI, roi.width / frame.size().width * 360);
}
