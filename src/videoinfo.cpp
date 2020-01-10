#include "videoinfo.hpp"

using namespace std;
namespace fs = std::filesystem;

VideoInfo::VideoInfo(string path, int split, int phi, int lambda) {
    this->path = path;

    fs::path p = fs::path(this->path);
    if ((p.extension().compare(".mp4") != 0) || (! fs::exists(p)))
        throw invalid_argument("Video file path invalid!");

    this->folder = p.parent_path();
    this->name = p.stem();

    cv::VideoCapture video = cv::VideoCapture(this->path);
    if (! video.isOpened())
        throw runtime_error("Could not open input video!");
    
    this->fps = static_cast<int>(round(video.get(cv::CAP_PROP_FPS)));
    this->width = static_cast<int>(round(video.get(cv::CAP_PROP_FRAME_WIDTH)));
    this->height = static_cast<int>(round(video.get(cv::CAP_PROP_FRAME_HEIGHT)));
    this->size = cv::Size(this->width, this->height);
    this->split = split;
    this->phi = phi;
    this->lambda = lambda;

    if (video.isOpened())
        video.release();
}

VideoInfo::VideoInfo(string path, int fps, const cv::Size &size, int split, int phi, int lambda) {
    this->path = path;
    fs::path p = fs::path(this->path);
    this->folder = p.parent_path();
    this->name = p.stem();

    this->fps = fps;
    this->width = size.width;
    this->height = size.height;
    this->size = size;
    this->split = split;
    this->phi = phi;
    this->lambda = lambda;
}
