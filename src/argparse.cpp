#include "argparse.hpp"

using namespace std;
namespace fs = std::filesystem;

const string ArgParse::INPUT_FOLDER = "data/input";

ArgParse::ArgParse(int argc, char **argv, Logger &log) {
    this->log = &log;
    // Move arguments to a vector
    for (int i = 1; i < argc; i++)
        this->args.push_back(argv[i]);
}

bool ArgParse::parse() {
    // Loop through all arguments
    for (int i = 0; i < this->args.size(); i++) {
        // Print help
        if ((this->args[i] == string("-h")) || (this->args[i] == string("--help"))) {
            this->log->info("Help will be added in future.");
            return false;
        }
        // Verbose output
        if ((this->args[i] == string("-v")) || (this->args[i] == string("--verbose"))) {
            this->verbose = true;
            this->log->setVerbose(true);
            continue;
        }
        // Method
        if ((this->args[i] == string("-m")) || (this->args[i] == string("--method"))) {
            if (this->args.size() < i + 2) {
                this->log->error(string("Argument ") + this->args[i] + string(" needs a value."));
                return false;
            }
            i += 1;
            if (this->args[i] == string("asuh")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_SUH;
                continue;
            }
            if (this->args[i] == string("aste")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_STE;
                continue;
            }
            if (this->args[i] == string("afan")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_FAN;
                continue;
            }
            if (this->args[i] == string("a360")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_360;
                continue;
            }
            if ((this->args[i] == string("gC3D")) || (this->args[i] == string("g"))) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_C3D;
                continue;
            }
            if (this->args[i] == string("gitt")) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_ITT;
                continue;
            }
            if (this->args[i] == string("gste")) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_STE;
                continue;
            }
            if (this->args[i] == string("gmar")) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_MAR;
                continue;
            }
            this->log->error(string("Invalid method ") + this->args[i]
                   + string(". Run with -h to show available methods."));
            return false;
        }
        // Skip steps
        if ((this->args[i] == string("-s")) || (this->args[i] == string("--skip-if-exists"))) {
            if (this->args.size() < i + 2) {
                this->log->error(string("Argument ") + this->args[i] + string(" needs a value."));
                return false;
            }
            i += 1;
            if (this->args[i] == string("g")) {
                this->skip = ArgParse::SKIP_GLIMPSES;
                continue;
            }
            this->log->error(string("Invalid value '") + this->args[i]
                   + string("' for skip if exists argument."));
            return false;
        }
        // Invalid argument
        this->log->error(string("Invalid argument ") + this->args[i] + string("."));
        return false;
    }

    // Check if method was chosen
    if (this->method == ArgParse::UNASSIGNED) {
        this->log->error("Parameter -m is required.");
        return false;
    }

    return true;
}

bool ArgParse::getInputs(vector<string> &input_paths) {
    // Check existence of input folder
    if (! fs::exists(ArgParse::INPUT_FOLDER)) {
        this->log->error("Non-existent input folder.");
        return false;
    }

    // Check if input folder contains any files
    if (fs::is_empty(ArgParse::INPUT_FOLDER)) {
        this->log->error("Empty input folder.");
        return false;
    }

    // Loop through files
    for (auto file : fs::directory_iterator("data/input")) {
        string ext = file.path().extension().string();
        // Check file type
        if ((ext == string(".mp4")) || (ext == string(".avi"))) {
            cv::VideoCapture test(file.path().string());
            // Check if codecs are available and video can be opened
            if (test.isOpened())
                input_paths.push_back(file.path());
        }
    }

    // Check if any video file found
    if (input_paths.empty()) {
        this->log->error("No valid video file in input folder.");
        return false;
    }

    return true;
}
