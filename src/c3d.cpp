#include "c3d.hpp"

using namespace std;
namespace fs = std::filesystem;

const string C3D::INPUT_LIST_NAME = "input_list.txt";
const string C3D::OUTPUT_PREFIX_NAME = "output_prefix.txt";
const string C3D::GLOG_CMD = "GLOG_logtosterr=1";
const string C3D::EXTRACTOR_PATH = "external/c3d/extract_image_features.bin";
const string C3D::PROTOTXT_PATH = "external/c3d/c3d_feature_extractor.prototxt";
const string C3D::MODEL_PATH = "external/c3d/trained_model";
const string C3D::TRAIN_FEATURES_DIR = "external/c3d/features";
const string C3D::GPU_ID = "0";
const vector<string> C3D::LAYERS = {"fc6-1", "fc7-1"};
const vector<string> C3D::CATEGORIES = {"hiking", "mountain_climbing", "parade", "soccer"};

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

    // Checking if main C3D output directory exists
    this->c3dPath = fs::path("data") / fs::path("c3d");
    fs::create_directories(this->c3dPath);

    this->loadFeatures(featuresPath, this->features);
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

bool C3D::evaluate(ScoreSpace &space, int category, int layer) {
    this->log->info("Evaluating capture-worthiness of computed C3D features.");

    // Load positive and negative training features
    this->log->debug("Loading training features for category '" + C3D::CATEGORIES[category]
                     + "'.");
    vector<Feature> neg_features;
    this->loadFeatures(fs::path(C3D::TRAIN_FEATURES_DIR)
                       / fs::path(C3D::CATEGORIES[category] + "_neg.c3d"), neg_features);
    vector<Feature> pos_features;
    this->loadFeatures(fs::path(C3D::TRAIN_FEATURES_DIR)
                       / fs::path(C3D::CATEGORIES[category] + "_pos.c3d"), pos_features);

    // Prepare training data
    int sample_count = neg_features.size() + pos_features.size();
    cv::Mat train_features(sample_count, C3D::FEATURE_LENGTH, CV_32F);
    cv::Mat train_labels(sample_count, 1, CV_32F);
    for (int i = 0; i < neg_features.size(); i++) {
        for (int j = 0; j < C3D::FEATURE_LENGTH; j++)
            train_features.at<float>(i, j) = neg_features[i][layer][j];
        train_labels.at<float>(i, 0) = 0;
    }
    for (int i = neg_features.size(); i < sample_count; i++) {
        for (int j = 0; j < C3D::FEATURE_LENGTH; j++)
            train_features.at<float>(i, j) = pos_features[i][layer][j];
        train_labels.at<float>(i, 0) = 1;
    }
    auto train_data = cv::ml::TrainData::create(train_features, cv::ml::ROW_SAMPLE, train_labels);

    // Train logistic regressor
    this->log->debug("Training logistic regressor.");
    auto regressor = cv::ml::LogisticRegression::create();
    regressor->setTrainMethod(cv::ml::LogisticRegression::MINI_BATCH);
    regressor->setMiniBatchSize(1);
    regressor->setIterations(sample_count);
    regressor->train(train_data);

    // Prepare features for prediction
    cv::Mat test_features(this->features.size(), C3D::FEATURE_LENGTH, CV_32F);
    for (int i = 0; i < neg_features.size(); i++) {
        for (int j = 0; j < C3D::FEATURE_LENGTH; j++)
            test_features.at<float>(i, j) = this->features[i][layer][j];
    }

    // Predict scores
    this->log->debug("Predicting scores.");
    cv::Mat scores(this->features.size(), 1, CV_32S);
    regressor->predict(test_features, scores);

    // Input values to score space
    for (int i = 0; i < this->features.size(); i++) {
        VideoInfo g = this->glimpses->get(i);
        space.set(g.split, g.phi, g.lambda, g.aov, scores.at<float>(i, 0));
    }

    return true;
}

void C3D::save() {
    fs::path featuresPath = this->c3dPath / fs::path(this->glimpses->videoName() + ".c3d");
    this->log->debug("Saving C3D features to file '" + featuresPath.string() + "'.");
    ofstream featuresFile(featuresPath, ios::binary);
    for (int i = 0; i < this->features.size(); i++) {
        for (int j = 0; j < C3D::LAYERS.size(); j++) {
            string desperate = "Writing feature '";
            for (int k = 0; k < this->features[i][j].size(); k++)
                desperate += to_string(this->features[i][j][k]) + " ";
            this->log->debug(desperate + "'.");
            featuresFile.write(reinterpret_cast<char*>(this->features[i][j].data()),
                               C3D::FEATURE_LENGTH * sizeof(float));
        }
    }
}

string C3D::concatStrings(const vector<string> &strings, const string separator) {
    std::string out;
    for (const auto &s : strings) out += s + separator;
    return out.substr(0, out.length() - separator.length());
}

void C3D::loadFeature(string featurePath, vector<float> &data) {
    ifstream feature(featurePath, ios::binary);
    
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

void C3D::loadFeatures(string featuresPath, vector<Feature> &features) {
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
        
        features.push_back(feature);
    }
}
