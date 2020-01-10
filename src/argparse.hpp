#ifndef __ARGPARSE__
#define __ARGPARSE__

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class ArgParse {

private:
    vector<string> args;

public:
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

    bool verbose = false;
    int method = UNASSIGNED;
    int submethod = UNASSIGNED;

    ArgParse(int argc, char **argv);
    string parse();

};

#endif // __ARGPARSE__