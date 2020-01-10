#ifndef __LOGGER__
#define __LOGGER__

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <ctime>

using namespace std;

class Logger {

private:
    static const string LOG_PATH;

    string getTime(bool date = false);

    bool verbose;
    ofstream file;

public:
    Logger(bool verbose = false, bool file = false);
    void debug(string message);
    void info(string message);
    void warning(string message);
    void error(string message);

};

#endif // __LOGGER__