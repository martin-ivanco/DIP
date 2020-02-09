#ifndef __ARGPARSE__
#define __ARGPARSE__

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "logger.hpp"

using namespace std;

class ArgParse {

private:
    vector<string> args;
    Logger *log;

    bool checkValue(int idx);

public:
    // Miscellaneous
    static const string INPUT_FOLDER;
    static const int UNASSIGNED = -1;

    // Methods
    static const int AUTOCROP = 0;
    static const int AUTOCROP_SUH = 0;
    static const int AUTOCROP_STE = 1;
    static const int AUTOCROP_FAN = 2;
    static const int AUTOCROP_360 = 3;
    
    static const int GLIMPSES = 1;
    static const int GLIMPSES_C3D = 0;
    static const int GLIMPSES_ITT = 1;
    static const int GLIMPSES_STE = 2;
    static const int GLIMPSES_MAR = 3;

    static const int DATASET = 2;
    static const int DATASET_2D = 0;
    static const int DATASET_3D = 1;

    // Categories
    static const int HIKING = 0;
    static const int MOUNTAIN_CLIMBING = 1;
    static const int PARADE = 2;
    static const int SOCCER = 3;

    // Debug skipping switches
    static const int SKIP_GLIMPSES = 1;
    static const int SKIP_SALIENCY = 2;

    bool verbose = false;
    int method = UNASSIGNED;
    int submethod = UNASSIGNED;
    int category = UNASSIGNED;
    int skip = 0;

    ArgParse(int argc, char **argv, Logger &log);
    bool parse();
    bool getInputs(vector<string> &input_paths);

};

#endif // __ARGPARSE__