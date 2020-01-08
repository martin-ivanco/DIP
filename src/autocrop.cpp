#include "autocrop.hpp"

using namespace std;

AutoCrop::AutoCrop(string videoFilePath, int saliencyType) {
    cv::VideoCapture video = cv::VideoCapture(videoFilePath);
    if (! video.isOpened())
        throw runtime_error("Couldn't open video.");

    cv::Mat frame;
    cv::Rect crop;
    int counter = 0;
    while (true) {
        video.read(frame);
        if (frame.empty())
            break;
        
        cerr << "Evaulating frame " << counter++ << endl;
        crop = this->getROI(frame, this->getSaliencyMap(frame, saliencyType), saliencyType);
        this->path.push_back(this->getCoords(crop));
    }

    this->saveToFile();
    cerr << "Finished." << endl;
}

vector<tuple<double, double, double>> AutoCrop::getPath() {
    return this->path;
}

cv::Mat AutoCrop::getSaliencyMap(cv::Mat frame, int type) {
    cerr << "Getting saliency" << endl;
    if (type == S_ITTI) {
        SalMapItti itti = SalMapItti(frame);
        return itti.salMap;
    }

    if (type == S_MARGOLIN) {
        SalMapMargolin margolin = SalMapMargolin(frame);
        return margolin.salMap;
    }

    if (type == S_STENTIFORD) {
        SalMapStentiford stentiford = SalMapStentiford(frame);
        stentiford.generateSalMap();
        return stentiford.salMap;
    }

    return cv::Mat();
}

cv::Rect AutoCrop::getROI(cv::Mat frame, cv::Mat saliency, int type) {
    cerr << "Getting ROI" << endl;
    if (type == S_ITTI) {
        AutocropSuh suh = AutocropSuh(saliency);
        suh.bruteForceWHratio(16, 9, 0.6f, 10, 10);
        return cv::Rect(suh.getX(), suh.getY(), suh.getWidth(), suh.getHeight());
    }

    if (type == S_MARGOLIN) {
        AutocropFang fang = AutocropFang(frame, saliency, FANG_MODEL_PATH);
        fang.WHratioCrop(16, 9, 1, 1);
        return cv::Rect(fang.getX(), fang.getY(), fang.getWidth(), fang.getHeight());
    }

    if (type == S_STENTIFORD) {
        AutocropStentiford stentiford = AutocropStentiford(saliency);
        stentiford.randomWHratio(16, 9, 1.5f);
        return cv::Rect(stentiford.getX(), stentiford.getY(),
            stentiford.getWidth(), stentiford.getHeight());
    }

    return cv::Rect();
}

tuple<double, double, double> AutoCrop::getCoords(cv::Rect roi) {
    double lam = (roi.x + roi.width / 2.0) * (2 * CV_PI) / 1280;
    double phi = (roi.y + roi.height / 2.0)* CV_PI / 720;
    return {phi - CV_PI / 2, lam - CV_PI, roi.width / 1280.0 * 360};
}

// for development only
void AutoCrop::saveToFile() {
    ofstream file("../Playground/crop_path.txt");
    for (int i = 0; i < this->path.size(); i++) {
        file << get<0>(this->path[i]) << " " << get<1>(this->path[i]) << " " << get<2>(this->path[i]) << "\n";
    }
}

// for development only
void AutoCrop::loadFromFile(string filePath) {

}

