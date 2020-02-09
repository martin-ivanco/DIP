#ifndef __C3D__
#define __C3D__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <opencv2/ml.hpp>

#include "glimpses.hpp"
#include "logger.hpp"
#include "scorespace.hpp"
#include "videoinfo.hpp"

using namespace std;

struct Feature {
    vector<float> fc6;
    vector<float> fc7;

    vector<float> &operator[](size_t idx) {
        if (idx == 0)
            return this->fc6;
        if (idx == 1)
            return this->fc7;
        throw out_of_range("Feature only has 2 layers. Index " + to_string(idx) + " out of range.");
    }
};

class C3D {

private:
    static const string INPUT_LIST_NAME;
    static const string OUTPUT_PREFIX_NAME;
    static const int SEGMENT_LENGTH = 16;
    static const string GLOG_CMD;
    static const string EXTRACTOR_PATH;
    static const string PROTOTXT_PATH;
    static const string MODEL_PATH;
    static const string TRAIN_FEATURES_DIR;
    static const string GPU_ID;
    static const int BATCH_SIZE = 25;
    static const vector<string> LAYERS;
    static const int FEATURE_LENGTH = 4096;
    static const vector<string> CATEGORIES;

    Logger *log;
    Glimpses *glimpses;
    vector<Feature> features;
    filesystem::path c3dPath;
    int segmentCount = 0;

    string concatStrings(const vector<string> &strings, const string separator = " ");
    void loadFeature(string featurePath, vector<float> &data);
    vector<float> computeMean(vector<vector<float>> &features);
    void loadFeatures(string featuresPath, vector<Feature> &features);

public:
    C3D(Glimpses &glimpses, Logger &log);
    C3D(string featuresPath, Logger &log);
    void prepare(bool use_splits = false);
    void extract();
    bool evaluate(ScoreSpace &space, int category, int layer = 0);
    void save();

};

#endif // __C3D__