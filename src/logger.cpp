#include "logger.hpp"

using namespace std;
namespace fs = std::filesystem;

const string Logger::LOG_PATH = "data/output";

Logger::Logger(bool verbose, bool file) {
    this->verbose = verbose;
    if (file) {
        fs::create_directories(Logger::LOG_PATH);
        this->file = ofstream(fs::path(Logger::LOG_PATH) / fs::path(this->getTime(true) + ".log"));
    }
}

void Logger::debug(string message) {
    if (this->verbose)
        cerr << message << endl;
    if (this->file.is_open())
        this->file << getTime() << " - DEBUG: " << message << endl;
}

void Logger::info(string message) {
    cout << message << endl;
    if (this->file.is_open())
        this->file << getTime() << " - INFO: " << message << endl;
}

void Logger::warning(string message) {
    cerr << "WARNING: " << message << endl;
    if (this->file.is_open())
        this->file << getTime() << " - WARNING: " << message << endl;
}

void Logger::error(string message) {
    cerr << "ERROR: " << message << endl;
    if (this->file.is_open())
        this->file << getTime() << " - ERROR: " << message << endl;
}

string Logger::getTime(bool date) {
    time_t epoch = std::time(nullptr);
    tm *tm_time = std::localtime(&epoch);
    char buffer[25];
    if (date) {
        std::sprintf (buffer, "%.4d-%.2d-%.2d_%.2d-%.2d-%.2d", tm_time->tm_year + 1900,
                      tm_time->tm_mon + 1, tm_time->tm_mday, tm_time->tm_hour, tm_time->tm_min,
                      tm_time->tm_sec);
    }
    else {
        std::sprintf (buffer, "%.2d:%.2d:%.2d", tm_time->tm_hour, tm_time->tm_min,
                      tm_time->tm_sec);
    }
    return buffer;
}
