#include "c3d.hpp"

using namespace std;
namespace fs = std::filesystem;

const string C3D::INPUT_LIST_NAME = "input_list.txt";
const string C3D::OUTPUT_PREFIX_NAME = "output_prefix.txt";
const string C3D::GLOG_CMD = "GLOG_logtosterr=1";
const string C3D::EXTRACTOR_PATH = "external/c3d/extract_image_features.bin";
const string C3D::PROTOTXT_PATH = "external/c3d/c3d_feature_extractor.prototxt";
const string C3D::MODEL_PATH = "external/c3d/trained_model";
const string C3D::GPU_ID = "0";
const string C3D::FEATURES = "fc6-1 fc7-1";

C3D::C3D(Glimpses &glimpses, Logger &log) {
    this->glimpses = &glimpses;
    this->log = &log;

    // Checking if main C3D output directory exists
    this->c3dPath = fs::path("data") / fs::path("c3d");
    fs::create_directories(this->c3dPath);
    this->prepare();
}

void C3D::prepare() {
    this->log->info("Preparing folders and lists for C3D feature extraction.");

    // Opening input and output lists for C3D feature extraction
    ofstream inputList(this->c3dPath / fs::path(C3D::INPUT_LIST_NAME));
    ofstream outputPrefix(this->c3dPath / fs::path(C3D::OUTPUT_PREFIX_NAME));

    // Creating C3D output directory for input video
    fs::path videoPath = this->c3dPath / fs::path(this->glimpses->videoName());
    fs::create_directories(videoPath);

    // Creating C3D output subdirectories for each glimpse
    char prefixBuffer[16];
    int glimpseSegmentCount = 0;
    for (int i = 0; i < this->glimpses->length(); i++) {
        fs::path glimpsePath = this->glimpses->get(i).path;
        fs::path glimpseFeaturesDirectory = videoPath / fs::path(this->glimpses->get(i).name);
        fs::create_directories(glimpseFeaturesDirectory);

        // Adding the glimpse to the lists
        glimpseSegmentCount = this->glimpses->get(i).length / C3D::SEGMENT_LENGTH;
        for (int j = 0; j < glimpseSegmentCount; j++) {
            inputList << glimpsePath.string() << " " << to_string(j * C3D::SEGMENT_LENGTH)
                       << " 0" << endl;
            sprintf(prefixBuffer, "%03d", j * C3D::SEGMENT_LENGTH);
            outputPrefix << (glimpseFeaturesDirectory / fs::path(prefixBuffer)).string()
                          << endl;
        }
        this->segmentCount += glimpseSegmentCount;
    }
}

void C3D::extract() {
    this->log->info("Extracting C3D features.");

    // Extracting C3D features
    string command = C3D::GLOG_CMD + " " + C3D::EXTRACTOR_PATH + " " + C3D::PROTOTXT_PATH + " "
                     + C3D::MODEL_PATH + " " + C3D::GPU_ID + " " + to_string(C3D::BATCH_SIZE)
                     + " " + to_string((this->segmentCount + C3D::BATCH_SIZE - 1) / C3D::BATCH_SIZE)
                     + " " + (this->c3dPath / fs::path(C3D::OUTPUT_PREFIX_NAME)).string() + " "
                     + C3D::FEATURES;
    system(command.c_str());
}
