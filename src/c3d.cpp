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
const vector<string> C3D::LAYERS = {"fc6-1", "fc7-1"};

C3D::C3D(Glimpses &glimpses, Logger &log) {
    this->glimpses = &glimpses;
    this->log = &log;

    // Checking if main C3D output directory exists
    this->c3dPath = fs::path("data") / fs::path("c3d");
    fs::create_directories(this->c3dPath);
}

C3D::C3D(string featuresPath, Logger &log) {
    this->glimpses = nullptr;
    this->log = &log;

    // Load features from file
    this->log->debug("Loading C3D features from file '" + featuresPath + "'.");
    ifstream featuresFile(featuresPath, ios::binary);
    while(! featuresFile.eof()) {
        Feature feature;

        for (int i = 0; i < C3D::LAYERS.size(); i++) {
            vector<float> temp;
            temp.reserve(C3D::FEATURE_LENGTH);
            featuresFile.read(reinterpret_cast<char*>(temp.data()),
                              C3D::FEATURE_LENGTH * sizeof(float));
            feature[i] = temp;
        }
        
        this->features.push_back(feature);
    }
}

void C3D::prepare(bool use_splits) {
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
    int length = use_splits ? this->glimpses->splitCount() : this->glimpses->length();
    for (int i = 0; i < length; i++) {
        fs::path glimpsePath = this->glimpses->get(i, use_splits).path;
        fs::path glimpseFeaturesDirectory = videoPath / fs::path(this->glimpses->get(i).name);
        fs::create_directories(glimpseFeaturesDirectory);

        // Adding the glimpse to the lists
        glimpseSegmentCount = this->glimpses->get(i, use_splits).length / C3D::SEGMENT_LENGTH;
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
                     + this->concatStrings(C3D::LAYERS);
    system(command.c_str());

    // Opening output list of C3D features
    ifstream outputList(this->c3dPath / fs::path(C3D::OUTPUT_PREFIX_NAME));

    // Counting mean feature for each glimpse
    this->log->info("Computing mean C3D features.");
    for (int i = 0; i < this->glimpses->length(); i++) {
        Feature feature;
        int glimpseSegmentCount = this->glimpses->get(i).length / C3D::SEGMENT_LENGTH;
        streampos filePos = outputList.tellg();

        for (int j = 0; j < C3D::LAYERS.size(); j++) {
            vector<vector<float>> layer;
            outputList.seekg(filePos);

            for (int k = 0; k < glimpseSegmentCount; k++) {
                string prefix;
                outputList >> prefix;

                vector<float> temp;
                this->loadFeature(prefix + "." + C3D::LAYERS[j], temp);
                layer.push_back(temp);
            }

            feature[j] = this->computeMean(layer);
        }

        this->features.push_back(feature);
    }

    // Delete the extraction folder
    fs::remove_all(this->c3dPath / fs::path(this->glimpses->videoName()));
}

bool C3D::evaluate(ScoreSpace &space, int category) {
    // TODO Load negative and positive examples
    // TODO Fit them to logistic regression
    // TODO Predict for each C3D feature the score and save it to space
    return true;
}

void C3D::save() {
    this->log->debug("Saving C3D features to file.");
    ofstream featuresFile(this->c3dPath / fs::path(this->glimpses->videoName() + ".c3d"),
                         ios::binary);
    for (int i = 0; i < this->features.size(); i++) {
        for (int j = 0; j < C3D::LAYERS.size(); j++)
            featuresFile.write(reinterpret_cast<char*>(this->features[i][j].data()),
                               C3D::FEATURE_LENGTH);
    }
}

string C3D::concatStrings(const vector<string> &strings, const string separator) {
    std::string out;
    for (const auto &s : strings) out += s + separator;
    return out.substr(0, out.length() - separator.length());
}

void C3D::loadFeature(string path, vector<float> &data) {
    ifstream feature(path, ios::binary);
    
    // Skip unimportant stuff
    feature.seekg(5 * sizeof(int));

    // Read the feature
    data.reserve(C3D::FEATURE_LENGTH);
    feature.read(reinterpret_cast<char*>(data.data()), C3D::FEATURE_LENGTH * sizeof(float));
}

vector<float> C3D::computeMean(vector<vector<float>> &features) {
    vector<float> mean;
    int dimension = features[0].size();
    mean.reserve(dimension);
    for (int i = 0; i < dimension; i++) {
        float sum = 0;
        for (auto f : features) sum += f[i];
        mean[i] = sum / features.size();
    }
    return mean;
}
