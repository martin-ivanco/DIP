#ifndef __C3D__
#define __C3D__

#include <iostream>
#include <string>
#include <fstream>

#include "glimpses.hpp"
#include "logger.hpp"

using namespace std;

class C3D {

private:
    static const string INPUT_LIST_NAME;
    static const string OUTPUT_PREFIX_NAME;
    static const int SEGMENT_LENGTH = 16;
    static const string GLOG_CMD;
    static const string EXTRACTOR_PATH;
    static const string PROTOTXT_PATH;
    static const string MODEL_PATH;
    static const string GPU_ID;
    static const int BATCH_SIZE = 25;
    static const string FEATURES;

    Logger *log;
    Glimpses *glimpses;
    filesystem::path c3dPath;
    int segmentCount = 0;

public:
    C3D(Glimpses &glimpses, Logger &log);
    void prepare(bool use_splits = false);
    void extract();

};

#endif // __C3D__