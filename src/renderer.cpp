#include "renderer.hpp"

using namespace std;
namespace fs = std::filesystem;

const int Renderer::FOURCC = cv::VideoWriter::fourcc('a', 'v', 'c', '1');

Renderer::Renderer(Logger &log) {
    this->log = &log;
}

vector<VideoInfo> Renderer::splitVideo(VideoInfo &video, string outputFolder, int splitLength,
                                       bool skipExisting) {
    this->log->info("Splitting video '" + video.path + "'.");
    vector<VideoInfo> splits;

    // Opening input video
    cv::VideoCapture reader;
    this->open(reader, video.path);
    cv::Mat frame;

    // Prepare for main loop
    cv::VideoWriter writer;
    bool finished = false;

    // Main loop
    while (! finished) {
        // Preparing split info
        string splitPath = fs::path(outputFolder) / this->getSplitName(splits.size());
        VideoInfo split(splitPath, 0, static_cast<double>(video.fps), video.size);

        // Opening split output
        if ((! skipExisting) || (! fs::exists(splitPath)))
            this->open(writer, splitPath, split.fps, split.size);
        this->log->debug("Generating split " + to_string(splits.size()) + ".");

        // Reading frames from input and writing to output for the length of split
        int i = 0;
        for (; i < video.fps * splitLength; i++) {
            reader.read(frame);
            if (frame.empty()) {
                finished = true;
                break;
            }
            if (writer.isOpened())
                writer.write(frame);
        }
        split.length = i;

        // Closing split output
        if (writer.isOpened())
            writer.release();

        splits.push_back(split);
    }

    // Closing all open videos
    this->close(reader, writer);
    return splits;
}

vector<VideoInfo> Renderer::composeViews(vector<VideoInfo> &videos, string outputFolder, int phi,
                                         int lambda, cv::Size &viewSize, bool skipExisting) {
    this->log->info("Composing views at phi=" + to_string(phi) + ", lambda=" + to_string(lambda)
                    + ".");
    vector<VideoInfo> views;

    // Checking if input vector is non-empty
    if (videos.size() < 1) {
        this->log->warning("Empty vector of videos provided.");
        return views;
    }

    // Preparing displacement maps and input/output variables
    auto [map1, map2] = this->getStereographicDisplacementMaps(videos[0].size, viewSize, phi, lambda);
    cv::VideoCapture reader;
    cv::Mat frame;
    cv::VideoWriter writer;
    cv::Mat warped;

    // Main loop
    for (int i = 0; i < videos.size(); i++) {
        // Opening input video and preparing view info
        this->open(reader, videos[i].path);
        string viewPath = fs::path(outputFolder) / this->getViewName(i, phi, lambda);
        VideoInfo view(viewPath, videos[i].length, static_cast<double>(videos[i].fps), viewSize, i, phi, lambda);

        // Opening view output and remapping each frame of input video
        if ((! skipExisting) || (! fs::exists(viewPath))) {
            this->open(writer, viewPath, view.fps, view.size);
            while (true) {
                if (! this->remapFrame(reader, writer, map1, map2, frame, warped))
                    break;
            }
        }

        views.push_back(view);
    }

    // Closing all open videos
    this->close(reader, writer);
    return views;
}

VideoInfo Renderer::renderTrajectory(VideoInfo &input,
                                     vector<tuple<double, double, double>> &trajectory,
                                     VideoInfo &output) {
    // Opening input and output videos
    cv::VideoCapture reader;
    this->open(reader, input.path);
    cv::Mat frame;
    cv::Mat warped;
    cv::VideoWriter writer;
    this->open(writer, output.path, output.fps, output.size);

    // Main loop - remap each frame using trajectory coordinates
    int i = 0;
    for (; i < trajectory.size(); i++) {
        auto [map1, map2] = this->getStereographicDisplacementMaps(input.size, output.size,
                                                                   get<0>(trajectory[i]), 
                                                                   get<1>(trajectory[i]),
                                                                   get<2>(trajectory[i]));
        if (! this->remapFrame(reader, writer, map1, map2, frame, warped)) {
            this->log->error("Video shorter than expected.");
            break;
        }
    }
    output.length = i;
    
    // Closing all open videos
    this->close(reader, writer);
    return output;
}

string Renderer::getSplitName(int timeBlock) {
    char buffer[100];
    sprintf (buffer, "s%.4d.mp4", timeBlock);
    return string(buffer);
}

string Renderer::getViewName(int timeBlock, int phi, int lambda) {
    char buffer[100];
    sprintf (buffer, "s%.4d_l%.3d_p%.3d.mp4", timeBlock, lambda, phi);
    return string(buffer);
}

tuple<cv::Mat, cv::Mat> Renderer::getStereographicDisplacementMaps(cv::Size &sourceSize,
                                                                   cv::Size &projectionSize,
                                                                   double phi, double lambda,
                                                                   double aov) {
    cv::Mat mapLam = cv::Mat(projectionSize.height, projectionSize.width, CV_32FC1);
    cv::Mat mapPhi = cv::Mat(projectionSize.height, projectionSize.width, CV_32FC1);
    int h = projectionSize.height / 2;
    int w = projectionSize.width / 2;
    double phi1 = this->deg2rad(phi);
    double lam0 = this->deg2rad(lambda);
    double R = w / tan(aov / 360 * CV_PI) / 2;
    double mSy = - 2 * R * tan(phi1);
    double mR = 2 * R / (cos(phi1) * sin(CV_PI / 2));

    for (int y = -h; y < h; y++) {
        for (int x = -w; x < w; x++) {
            double ro = sqrt(x * x + y * y);
            if (ro == 0) {
                mapPhi.at<float>(y + h, x + w) = static_cast<float>(sourceSize.height / 2.0 - 0.5);
                mapLam.at<float>(y + h, x + w) = static_cast<float>(sourceSize.width / 2.0 - 0.5);
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

            auto [erpY, erpX] = this->rad2erp(sourceSize, rPhi, rLam);
            mapPhi.at<float>(y + h, x + w) = static_cast<float>(erpY);
            mapLam.at<float>(y + h, x + w) = static_cast<float>(erpX);
        }
    }

    cv::Mat map1 = cv::Mat(projectionSize.height, projectionSize.width, CV_16SC2);
    cv::Mat map2 = cv::Mat(projectionSize.height, projectionSize.width, CV_16UC1);
    cv::convertMaps(mapLam, mapPhi, map1, map2, CV_16SC2);
    return {map1, map2};
}

double Renderer::deg2rad(double deg) {
    return deg / 180.0 * CV_PI;
}

tuple<double, double> Renderer::rad2erp(cv::Size planeSize, double phi, double lambda) {
    phi += CV_PI / 2;
    lambda = fmod(lambda + CV_PI, 2 * CV_PI);
    lambda = lambda < 0 ? lambda + 2 * CV_PI : lambda;
    return {phi / CV_PI * planeSize.height, lambda / (2 * CV_PI) * planeSize.width};
}

bool Renderer::remapFrame(cv::VideoCapture &capture, cv::VideoWriter &writer, cv::Mat &map1, cv::Mat &map2, cv::Mat &frame, cv::Mat &warped) {
    capture.read(frame);
    if (frame.empty())
        return false;
    cv::remap(frame, warped, map1, map2, cv::INTER_LINEAR);
    writer.write(warped);
    return true;
}

void Renderer::open(cv::VideoCapture &capture, string filename) {
    capture.open(filename);
    if (! capture.isOpened())
        throw runtime_error("Could not open input video!");
}

void Renderer::open(cv::VideoWriter &writer, string filename, int fps, cv::Size &size) {
    writer.open(filename, Renderer::FOURCC, fps, size);
    if (! writer.isOpened())
        throw runtime_error("Could not write video!");
}

void Renderer::close(cv::VideoCapture &capture, cv::VideoWriter &writer) {
    if (writer.isOpened())
        writer.release();
    if (capture.isOpened())
        capture.release();
}
