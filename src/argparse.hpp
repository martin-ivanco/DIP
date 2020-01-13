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

public:
    static const string INPUT_FOLDER;
    static const int UNASSIGNED = -1;

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

    static const int SKIP_GLIMPSES = 0;

    bool verbose = false;
    int method = UNASSIGNED;
    int submethod = UNASSIGNED;
    int skip = UNASSIGNED;

    ArgParse(int argc, char **argv, Logger &log);
    bool parse();
    bool getInputs(vector<string> &input_paths);

};

#endif // __ARGPARSE__