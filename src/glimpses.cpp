#include "glimpses.hpp"

using namespace std;
namespace fs = std::filesystem;

Glimpses::Glimpses(string videoFilePath) : originalVideo(videoFilePath) {
    fs::path p = fs::path(this->originalVideo.folder);
    this->folderPath = p / this->originalVideo.name;
    if ((! fs::exists(this->folderPath)) && (! fs::create_directory(this->folderPath)))
        throw runtime_error("Could not create directory!");

    this->generateGlimpses();
}

int Glimpses::length() {
    return this->glimpses.size();
}

VideoInfo Glimpses::get(int index) {
    return this->glimpses.at(index);
}

void Glimpses::generateGlimpses() {
    this->splitVideo();
    
    for (auto p : PHIS) {
        for (auto l : LAMBDAS) {
            this->composeView(p, l);
        }
    }
}

void Glimpses::splitVideo() {
    cv::VideoCapture video = cv::VideoCapture(this->originalVideo.path);
    if (! video.isOpened())
        throw runtime_error("Could not open input video!");
    cv::Mat frame;

    cv::VideoWriter writer;
    int fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');

    bool finished = false;
    while (! finished)
    {
        string splitPath = fs::path(this->folderPath) / this->getSplitName(this->splits.size());
        VideoInfo split = VideoInfo(splitPath, static_cast<double>(this->originalVideo.fps), this->originalVideo.size);
        
        writer.open(splitPath, fourcc, split.fps, split.size);
        if (! writer.isOpened())
            throw runtime_error("Could not write video!");

        for (int i = 0; i < this->originalVideo.fps * 5; i++) {
            video.read(frame);
            if (frame.empty()) {
                finished = true;
                break;
            }
            writer.write(frame);
        }

        this->splits.push_back(split);
    }

    if (writer.isOpened())
        writer.release();
    if (video.isOpened())
        video.release();
}

void Glimpses::composeView(int phi, int lambda) {
    // auto [mapLam, mapPhi] = this->getStereographicDisplacementMaps(phi, lambda);
    // cv::Mat map1 = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_16SC2);
    // cv::Mat map2 = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_16UC1);
    // cv::convertMaps(mapLam, mapPhi, map1, map2, CV_16SC2);

    // cv::VideoCapture clip;
    // cv::Mat frame;
    // cv::VideoWriter writer;
    // cv::Mat warped;
    // int fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');

    for (int i = 0; i < this->splits.size(); i++) {
        // clip.open(splits[i].path);
        // if (! clip.isOpened())
        //     throw runtime_error("Could not open split!");
        
        string glimpsePath = fs::path(this->folderPath) / this->getGlimpseName(i, phi, lambda);
        VideoInfo glimpse = VideoInfo(glimpsePath, static_cast<double>(this->originalVideo.fps), cv::Size(GLIMPSE_WIDTH, GLIMPSE_HEIGHT));

        // writer.open(glimpsePath, fourcc, glimpse.fps, glimpse.size);
        // if (! writer.isOpened())
        //     throw runtime_error("Could not write video!");

        // while (true) {
        //     clip.read(frame);
        //     if (frame.empty())
        //         break;
        //     cv::remap(frame, warped, map1, map2, cv::INTER_LINEAR);
        //     writer.write(warped);
        // }

        this->glimpses.push_back(glimpse);
    }

    // if (writer.isOpened())
    //     writer.release();
    // if (clip.isOpened())
    //     clip.release();
}

string Glimpses::getSplitName(int timeBlock) {
    char buffer[100];
    sprintf (buffer, "%s_g%.4d.mp4", this->originalVideo.name.c_str(), timeBlock);
    return string(buffer);
}

string Glimpses::getGlimpseName(int timeBlock, int phi, int lambda) {
    char buffer[100];
    sprintf (buffer, "%s_g%.4d_h%.3d_v%.3d.mp4", this->originalVideo.name.c_str(), timeBlock, lambda, phi);
    return string(buffer);
}

tuple<cv::Mat, cv::Mat> Glimpses::getGnomonicDisplacementMaps(int phi, int lambda) {
    cv::Mat mapLam = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_32FC1);
    cv::Mat mapPhi = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_32FC1);
    int h = GLIMPSE_HEIGHT / 2;
    int w = GLIMPSE_WIDTH / 2;
    double phi1 = this->deg2rad(phi);
    double lam0 = this->deg2rad(lambda);
    double aov = 65.5;
    double R = w / tan(aov / 360 * CV_PI);

    for (int y = -h; y < h; y++) {
        for (int x = -w; x < w; x++) {
            double ro = sqrt(x * x + y * y);
            double c = atan(ro / R);

            double rPhi = asin(cos(c) * sin(phi1) + (y * sin(c) * cos(phi1) / ro));
            double rLam;
            if (phi == 90)
                rLam = lam0 + atan(static_cast<double>(x) / (-y));
            else if (phi == -90)
                rLam = lam0 + atan(static_cast<double>(x) / y);
            else
                rLam = lam0 + atan(x * sin(c) / (ro * cos(phi1) * cos(c) - y * sin(phi1) * sin(c)));

            auto [erpY, erpX] = this->rad2erp(rPhi, rLam);
            mapPhi.at<float>(y + h, x + w) = static_cast<float>(erpY);
            mapLam.at<float>(y + h, x + w) = static_cast<float>(erpX);
        }
    }
    
    return {mapLam, mapPhi};
}

tuple<cv::Mat, cv::Mat> Glimpses::getStereographicDisplacementMaps(int phi, int lambda) {
    cv::Mat mapLam = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_32FC1);
    cv::Mat mapPhi = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_32FC1);
    int h = GLIMPSE_HEIGHT / 2;
    int w = GLIMPSE_WIDTH / 2;
    double phi1 = this->deg2rad(phi);
    double lam0 = this->deg2rad(lambda);
    double aov = 104.3;
    double R = w / tan(aov / 360 * CV_PI) / 2;
    double mSy = - 2 * R * tan(phi1);
    double mR = 2 * R / (cos(phi1) * sin(CV_PI / 2));

    for (int y = -h; y < h; y++) {
        for (int x = -w; x < w; x++) {
            double ro = sqrt(x * x + y * y);
            if (ro == 0) {
                mapPhi.at<float>(y + h, x + w) = static_cast<float>(this->originalVideo.height / 2.0 - 0.5);
                mapLam.at<float>(y + h, x + w) = static_cast<float>(this->originalVideo.width / 2.0 - 0.5);
                continue;
            }
            double c = 2 * atan(ro / (2 * R));

            double rPhi = asin(cos(c) * sin(phi1) + (y * sin(c) * cos(phi1) / ro));
            double rLam;
            if (phi == 90)
                rLam = lam0 + atan(static_cast<double>(x) / (-y));
            else if (phi == -90)
                rLam = lam0 + atan(static_cast<double>(x) / y);
            else
                rLam = lam0 + atan(x * sin(c) / (ro * cos(phi1) * cos(c) - y * sin(phi1) * sin(c)));

            if (pow(x, 2) + pow(y - mSy, 2) > pow(mR, 2))
                rLam += CV_PI;

            auto [erpY, erpX] = this->rad2erp(rPhi, rLam);
            mapPhi.at<float>(y + h, x + w) = static_cast<float>(erpY);
            mapLam.at<float>(y + h, x + w) = static_cast<float>(erpX);
        }
    }

    return {mapLam, mapPhi};
}

double Glimpses::deg2rad(double deg) {
    return deg / 180.0 * CV_PI;
}

tuple<double, double> Glimpses::rad2erp(double phi, double lambda) {
    phi += CV_PI / 2;
    lambda = fmod(lambda + CV_PI, 2 * CV_PI);
    lambda = lambda < 0 ? lambda + 2 * CV_PI : lambda;
    return {phi / CV_PI * this->originalVideo.height, lambda / (2 * CV_PI) * this->originalVideo.width};
}

// for development only
Glimpses::Glimpses() : originalVideo("../Playground/test.mp4") {
    fs::path p = fs::path(this->originalVideo.folder);
    this->folderPath = p / this->originalVideo.name;

    for (int i = 0; i < 6; i++) {
        string splitPath = fs::path(this->folderPath) / this->getSplitName(this->splits.size());
        VideoInfo split = VideoInfo(splitPath, static_cast<double>(this->originalVideo.fps), this->originalVideo.size);
        this->splits.push_back(split);
    }
    
    for (auto p : PHIS) {
        for (auto l : LAMBDAS) {
            for (int i = 0; i < this->splits.size(); i++) { 
                string glimpsePath = fs::path(this->folderPath) / this->getGlimpseName(i, p, l);
                VideoInfo glimpse = VideoInfo(glimpsePath, static_cast<double>(this->originalVideo.fps), cv::Size(GLIMPSE_WIDTH, GLIMPSE_HEIGHT));
                this->glimpses.push_back(glimpse);
            }
        }
    }
}