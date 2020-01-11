#include "argparse.hpp"

using namespace std;
namespace fs = std::filesystem;

const string ArgParse::INPUT_FOLDER = "data/input";

ArgParse::ArgParse(int argc, char **argv) {
    for (int i = 1; i < argc; i++)
        this->args.push_back(argv[i]);
}

string ArgParse::parse() {
    for (int i = 0; i < this->args.size(); i++) {
        if ((this->args[i] == string("-h")) || (this->args[i] == string("--help"))) {
            return "TODO help.";
        }
        if ((this->args[i] == string("-v")) || (this->args[i] == string("--verbose"))) {
            this->verbose = true;
            continue;
        }
        if ((this->args[i] == string("-m")) || (this->args[i] == string("--method"))) {
            if (this->args.size() < i + 2)
                return string("Argument ") + this->args[i] + string(" needs a value.");
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
            return string("Invalid method ") + this->args[i]
                   + string(". Run with -h to show available methods.");
        }
        if ((this->args[i] == string("-s")) || (this->args[i] == string("--skip-if-exists"))) {
            if (this->args.size() < i + 2)
                return string("Argument ") + this->args[i] + string(" needs a value.");
            i += 1;
            if (this->args[i] == string("g")) {
                this->skip = ArgParse::SKIP_GLIMPSES;
                continue;
            }
            return string("Invalid value '") + this->args[i]
                   + string("' for skip if exists argument.");
        }
        return string("Invalid argument ") + this->args[i] + string(".");
    }

    if (this->method == ArgParse::UNASSIGNED)
        return "Parameter -m is required.";

    return "";
}

string ArgParse::getInputs(vector<string> &input_paths) {
    if (! fs::exists(ArgParse::INPUT_FOLDER))
        return "Non-existent input folder.";

    if (fs::is_empty(ArgParse::INPUT_FOLDER))
        return "Empty input folder.";

    for (auto file : fs::directory_iterator("data/input")) {
        string ext = file.path().extension().string();
        if ((ext == string(".mp4")) || (ext == string(".avi"))) {
            cv::VideoCapture test(file.path().string());
            if (test.isOpened())
                input_paths.push_back(file.path());
        }
    }

    if (input_paths.empty())
        return "No valid video file in input folder.";

    return "";
}
