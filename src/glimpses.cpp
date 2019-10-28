#include "glimpses.hpp"

using namespace std;
namespace fs = std::filesystem;

Glimpses::Glimpses(string videoFilePath) {
    this->originalPath = videoFilePath;
    fs::path p = fs::path(this->originalPath);
    if ((p.extension().compare(".mp4") != 0) || (! fs::exists(p)))
        throw invalid_argument("videoFilePath");

    this->name = p.stem();
    // TODO check if already exists
    if (! fs::create_directory(p.parent_path() / p.stem()))
        throw runtime_error("Could not create directory!");

    this->folderPath = p.parent_path() / p.stem();

    this->generateGlimpses();
}

void Glimpses::generateGlimpses() {
    this->splitVideo();
}

void Glimpses::splitVideo() {
    cv::VideoCapture video = cv::VideoCapture(this->originalPath);
    if (! video.isOpened())
        throw runtime_error("Could not open input video!");

    cv::Mat frame;
    int fps = (int) round(video.get(cv::CAP_PROP_FPS));
    cv::Size size = cv::Size((int) round(video.get(cv::CAP_PROP_FRAME_WIDTH)), (int) round(video.get(cv::CAP_PROP_FRAME_HEIGHT)));
    cv::VideoWriter writer;
    int counter = 0;
    bool finished = false;

    while (! finished)
    {
        string filename = fs::path(this->folderPath) / this->getGlimpseName(counter, -500, -500);
        writer.open(filename, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), (double) fps, size);

        if (! writer.isOpened())
            throw runtime_error("Could not write video!");

        for (size_t i = 0; i < fps * 5; i++) {
            video.read(frame);
            if (frame.empty()) {
                finished = true;
                break;
            }
            writer.write(frame);
        }

        counter++;
    }

    if (writer.isOpened())
        writer.release();
}

void Glimpses::composeView() {
    
}

string Glimpses::getGlimpseName(int timeBlock, int longitude, int latitude) {
    char buffer[100];
    if (longitude < -400)
        sprintf (buffer, "%s_g%.4d.mp4", this->name.c_str(), timeBlock);
    else
        sprintf (buffer, "%s_g%.4d_h%.3d_v%.3d.mp4", this->name.c_str(), timeBlock, longitude, latitude);
    return string(buffer);
}

void Glimpses::bilinear() {
    
}