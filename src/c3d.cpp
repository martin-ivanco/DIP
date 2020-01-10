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

C3D::C3D(Glimpses glimpses): glimpses(glimpses) {
    this->c3d_path = fs::path("data") / fs::path("c3d");
    fs::create_directories(this->c3d_path);
    this->prepare();
    this->extract();
}

void C3D::prepare() {
    ofstream input_list(this->c3d_path / fs::path(C3D::INPUT_LIST_NAME));
    ofstream output_prefix(this->c3d_path / fs::path(C3D::OUTPUT_PREFIX_NAME));

    fs::path video_path = this->c3d_path / fs::path(this->glimpses.getOriginalVideo().name);
    fs::create_directories(video_path);

    char prefix_buffer[16];
    for (int i = 0; i < this->glimpses.length(); i++) {
        fs::path glimpse_path = this->glimpses.get(i).path;
        fs::path glimpse_features_directory = video_path / fs::path(this->glimpses.get(i).name);
        fs::create_directories(glimpse_features_directory);

        for (int j = 0; j < C3D::SEGMENT_COUNT; j++) {
            input_list << glimpse_path.string() << " " << to_string(j * C3D::SEGMENT_LENGTH) << " 0" << endl;
            sprintf(prefix_buffer, "%03d", j * C3D::SEGMENT_LENGTH);
            output_prefix << (glimpse_features_directory / fs::path(prefix_buffer)).string() << endl;
        }
    }
}

void C3D::extract() {
    string command = C3D::GLOG_CMD + " " + C3D::EXTRACTOR_PATH + " " + C3D::PROTOTXT_PATH + " "
                     + C3D::MODEL_PATH + " " + C3D::GPU_ID + " " + to_string(C3D::BATCH_SIZE)
                     + " " + to_string((this->glimpses.length() * C3D::SEGMENT_COUNT
                                       + C3D::BATCH_SIZE - 1) / C3D::BATCH_SIZE) + " "
                     + (this->c3d_path / fs::path(C3D::OUTPUT_PREFIX_NAME)).string() + " "
                     + C3D::FEATURES;
    system(command.c_str());
}
