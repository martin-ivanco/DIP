#ifndef __LOGGER__
#define __LOGGER__

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

class Logger {

private:
    static const string LOG_PATH;

    string getTime(bool date = false);

    bool verbose = false;
    ofstream file;

public:
    Logger(bool file = false);
    void setVerbose(bool value);
    void debug(string message);
    void info(string message);
    void warning(string message);
    void error(string message);

};

#endif // __LOGGER__