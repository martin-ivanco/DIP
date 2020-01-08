#ifndef __C3D__
#define __C3D__

#include <iostream>
#include <string>
#include <fstream>

#include "glimpses.hpp"
#include "constants.hpp"
#include "tools.hpp"

using namespace std;

class C3D {

private:
    static const string INPUT_LIST_NAME;
    static const string OUTPUT_PREFIX_NAME;
    static const int SEGMENT_COUNT;
    static const int SEGMENT_LENGTH;
    static const string GLOG_CMD;
    static const string EXTRACTOR_PATH;
    static const string PROTOTXT_PATH;
    static const string MODEL_PATH;
    static const string GPU_ID;
    static const int BATCH_SIZE;
    static const string FEATURES;

    filesystem::path c3d_path;
    Glimpses glimpses;
    
    void prepare();
    void extract();

public:
    C3D(Glimpses glimpses);

};

#endif // __C3D__