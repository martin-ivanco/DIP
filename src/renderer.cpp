#include "renderer.hpp"

using namespace std;
namespace fs = std::filesystem;

Renderer::Renderer(string videoFilePath) : originalVideo(videoFilePath) {
    this->folderPath = fs::path("data/glimpses") / this->originalVideo.name;
    if ((! fs::exists(this->folderPath)) && (! fs::create_directories(this->folderPath)))
        throw runtime_error("Could not create directory!");
}

vector<VideoInfo> Renderer::splitVideo(int splitLength) {
    vector<VideoInfo> splits;

    cv::VideoCapture video;
    this->open(video, this->originalVideo.path);
    cv::Mat frame;

    cv::VideoWriter writer;
    bool finished = false;
    while (! finished)
    {
        string splitPath = fs::path(this->folderPath) / this->getSplitName(splits.size());
        VideoInfo split = VideoInfo(splitPath, static_cast<double>(this->originalVideo.fps), this->originalVideo.size);
        this->open(writer, splitPath, split.fps, split.size);

        for (int i = 0; i < this->originalVideo.fps * splitLength; i++) {
            video.read(frame);
            if (frame.empty()) {
                finished = true;
                break;
            }
            writer.write(frame);
        }

        splits.push_back(split);
    }

    this->close(video, writer);
    return splits;
}

vector<VideoInfo> Renderer::composeViews(int phi, int lambda, vector<VideoInfo> videos) {
    vector<VideoInfo> views;
    auto [map1, map2] = this->getStereographicDisplacementMaps(phi, lambda);
    cv::VideoCapture clip;
    cv::Mat frame;
    cv::VideoWriter writer;
    cv::Mat warped;

    for (int i = 0; i < videos.size(); i++) {
        this->open(clip, videos[i].path);
        
        string viewPath = fs::path(this->folderPath) / this->getViewName(i, phi, lambda);
        VideoInfo view = VideoInfo(viewPath, static_cast<double>(this->originalVideo.fps), cv::Size(GLIMPSE_WIDTH, GLIMPSE_HEIGHT), i, phi, lambda);
        this->open(writer, viewPath, view.fps, view.size);

        while (true) {
            if (! this->remapFrame(clip, writer, map1, map2, frame, warped))
                break;
        }

        views.push_back(view);
    }

    this->close(clip, writer);
    return views;
}

VideoInfo Renderer::renderSplitPath(vector<tuple<int, int>> path) {
    cv::VideoCapture video;
    this->open(video, this->originalVideo.path);
    
    cv::VideoWriter writer;
    string outputPath = fs::path(this->folderPath) / "output.mp4";
    VideoInfo output = VideoInfo(outputPath, static_cast<double>(this->originalVideo.fps), cv::Size(GLIMPSE_WIDTH, GLIMPSE_HEIGHT));
    this->open(writer, outputPath, output.fps, output.size);

    auto [map1, map2] = this->getStereographicDisplacementMaps(PHIS[get<0>(path[0])], LAMBDAS[get<1>(path[0])]);
    cv::Mat frame;
    cv::Mat warped;
    
    for (int i = 0; i < (SPLIT_LENGTH_SECONDS / 2.0) * originalVideo.fps; i++) {
        if (! this->remapFrame(video, writer, map1, map2, frame, warped))
            throw runtime_error("Video shorter than expected.");
    }

    for (int s = 1; s < path.size(); s++) {
        int startPhi = PHIS[get<0>(path[s - 1])];
        int startLambda = LAMBDAS[get<1>(path[s - 1])];
        int diffPhi = PHIS[get<0>(path[s])] - startPhi;
        int diffLambda = LAMBDAS[get<1>(path[s])] - startLambda;
        double progress = 0;

        for (int i = 0; i < SPLIT_LENGTH_SECONDS * originalVideo.fps; i++) {
            if ((diffPhi != 0) || (diffLambda != 0)) {
                progress = static_cast<double>(i) / (SPLIT_LENGTH_SECONDS * originalVideo.fps);
                tie(map1, map2) = this->getStereographicDisplacementMaps(progress * diffPhi + startPhi, progress * diffLambda + startLambda);
            }

            if (! this->remapFrame(video, writer, map1, map2, frame, warped))
                throw runtime_error("Video shorter than expected.");
        }
    }

    tie(map1, map2) = this->getStereographicDisplacementMaps(PHIS[get<0>(path[path.size() - 1])], LAMBDAS[get<1>(path[path.size() - 1])]);

    while (true) {
        if (! this->remapFrame(video, writer, map1, map2, frame, warped))
            break;
    }
    
    this->close(video, writer);
    return output;
}

VideoInfo Renderer::renderPath(vector<tuple<double, double, double>> path) {
    cv::VideoCapture video;
    this->open(video, this->originalVideo.path);
    
    cv::VideoWriter writer;
    string outputPath = fs::path(this->folderPath) / "output.mp4";
    VideoInfo output = VideoInfo(outputPath, static_cast<double>(this->originalVideo.fps), cv::Size(GLIMPSE_WIDTH, GLIMPSE_HEIGHT));
    this->open(writer, outputPath, output.fps, output.size);

    cv::Mat frame;
    cv::Mat warped;
    for (int i = 0; i < path.size(); i++) {
        auto [map1, map2] = this->getStereographicDisplacementMaps(get<0>(path[0]), get<1>(path[0]), get<2>(path[0]));
        if (! this->remapFrame(video, writer, map1, map2, frame, warped))
            throw runtime_error("Video shorter than expected.");
    }
    
    this->close(video, writer);
    return output;
}

string Renderer::getSplitName(int timeBlock) {
    char buffer[100];
    sprintf (buffer, "%s_g%.4d.mp4", this->originalVideo.name.c_str(), timeBlock);
    return string(buffer);
}

string Renderer::getViewName(int timeBlock, int phi, int lambda) {
    char buffer[100];
    sprintf (buffer, "%s_g%.4d_h%.3d_v%.3d.mp4", this->originalVideo.name.c_str(), timeBlock, lambda, phi);
    return string(buffer);
}

tuple<cv::Mat, cv::Mat> Renderer::getStereographicDisplacementMaps(double phi, double lambda, double aov) {
    cv::Mat mapLam = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_32FC1);
    cv::Mat mapPhi = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_32FC1);
    int h = GLIMPSE_HEIGHT / 2;
    int w = GLIMPSE_WIDTH / 2;
    double phi1 = this->deg2rad(phi);
    double lam0 = this->deg2rad(lambda);
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

    cv::Mat map1 = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_16SC2);
    cv::Mat map2 = cv::Mat(GLIMPSE_HEIGHT, GLIMPSE_WIDTH, CV_16UC1);
    cv::convertMaps(mapLam, mapPhi, map1, map2, CV_16SC2);
    return {map1, map2};
}

double Renderer::deg2rad(double deg) {
    return deg / 180.0 * CV_PI;
}

tuple<double, double> Renderer::rad2erp(double phi, double lambda) {
    phi += CV_PI / 2;
    lambda = fmod(lambda + CV_PI, 2 * CV_PI);
    lambda = lambda < 0 ? lambda + 2 * CV_PI : lambda;
    return {phi / CV_PI * this->originalVideo.height, lambda / (2 * CV_PI) * this->originalVideo.width};
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

void Renderer::open(cv::VideoWriter &writer, string filename, int fps, cv::Size size) {
    writer.open(filename, FOURCC_DEFAULT, fps, size);
    if (! writer.isOpened())
        throw runtime_error("Could not write video!");
}

void Renderer::close(cv::VideoCapture &capture, cv::VideoWriter &writer) {
    if (writer.isOpened())
        writer.release();
    if (capture.isOpened())
        capture.release();
}

// for development only
VideoInfo Renderer::getVideoInfo() {
    return originalVideo;
}
