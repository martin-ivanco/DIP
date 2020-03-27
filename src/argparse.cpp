#include "argparse.hpp"

using namespace std;
namespace fs = std::filesystem;

const string ArgParse::INPUT_FOLDER = "data/input";
const string ArgParse::HELP_MESSAGE = 
    "usage: ./run.sh run [-h] [-v] -m <method> -c <category> [-s] [-t <number>]\n"
    "\n"
    "\e[1mAutomatic Spherical Video Cropping Algorithms\e[0m\n"
    "\n"
    "This application implements several methods of cropping spherical videos to\n"
    "normal field of view videos. The input videos need to be stored in the\n"
    "'data/input' folder. The output will be rendered into 'data/output' folder.\n"
    "\n"
    "required arguments:\n"
    "  -m, --method     automatc cropping method to use - available methods are:\n"
    "    ac-xxx         basic automatic cropping methods applied on each frame\n"
    "                   separately without temporal coherence\n"
    "      ac-suh       utilizes method by Suh et. al. implemented by Ambrož\n"
    "      ac-ste       utilizes method by Stentiford implemented by Ambrož\n"
    "      ac-fan       utilizes method by Fang et. al. implemented by Ambrož\n"
    "    stg-xxx        methods based on Pano2Vid by Su et. al. using\n"
    "                   spatio-temporal glimpses\n"
    "      stg-c3d      original method using classification based on C3D features\n"
    "      stg-itt      evaluates scores using saliency mapping by Itti et. al.\n"
    "      stg-ste      evaluates scores using saliency mapping by Stentiford\n"
    "      stg-mar      evaluates scores using saliency mapping by Margolin et. al.\n"
    "    aid-xxxx       methods based on Shot Orientation Controls for Interactive\n"
    "                   Cinematography with 360° Video by Pavel et. al.\n"
    "      aid-shot     original method as explained in section Automatic Importance\n"
    "                   Detection - static view for each shot\n"
    "      aid-cont     reorients camera for each frame - using --smooth is highly\n"
    "                   recommended\n"
    "    ds-xx          C3D features dataset preparation methods\n"
    "      ds-2D        for classic 2D videos\n"
    "      ds-3D        for spherical videos\n"
    "\n"
    "optional arguments:\n"
    "  -h, --help       show this help message and exit\n"
    "  -v, --verbose    print debug messages\n"
    "  -c, --category   category of spatial video, required for gC3D method -\n"
    "                   available categories are: hiking, mountain_climbing,\n"
    "                   parade, soccer\n"
    "  -s, --smooth     smooth the acquired trajectory before rendering video -\n"
    "                   useful for automatic cropping methods and aid-cont method\n"
    "  -t, --threads    if OpenMP is supported on current machine, this argument\n"
    "                   specifies, how many threads should be used to process input\n"
    "                   in parallel\n";

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
            this->log->info(ArgParse::HELP_MESSAGE);
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
            if (! this->checkValue(i))
                return false;
            i += 1;
            if (this->args[i] == string("ac-suh")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_SUH;
                continue;
            }
            if (this->args[i] == string("ac-ste")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_STE;
                continue;
            }
            if (this->args[i] == string("ac-fan")) {
                this->method = ArgParse::AUTOCROP;
                this->submethod = ArgParse::AUTOCROP_FAN;
                continue;
            }
            if (this->args[i] == string("stg-c3d")){
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_C3D;
                continue;
            }
            if (this->args[i] == string("stg-itt")) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_ITT;
                continue;
            }
            if (this->args[i] == string("stg-ste")) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_STE;
                continue;
            }
            if (this->args[i] == string("stg-mar")) {
                this->method = ArgParse::GLIMPSES;
                this->submethod = ArgParse::GLIMPSES_MAR;
                continue;
            }
            if (this->args[i] == string("aid-shot")) {
                this->method = ArgParse::AID;
                this->submethod = ArgParse::AID_SHOT;
                continue;
            }
            if (this->args[i] == string("aid-cont")) {
                this->method = ArgParse::AID;
                this->submethod = ArgParse::AID_CONT;
                continue;
            }
            if (this->args[i] == string("ds-2D")) {
                this->method = ArgParse::DATASET;
                this->submethod = ArgParse::DATASET_2D;
                continue;
            }
            if (this->args[i] == string("ds-3D")) {
                this->method = ArgParse::DATASET;
                this->submethod = ArgParse::DATASET_3D;
                continue;
            }
            this->log->error(string("Invalid method '") + this->args[i]
                   + string("'. Run with -h to show available methods."));
            return false;
        }

        // Category
        if ((this->args[i] == string("-c")) || (this->args[i] == string("--category"))) {
            if (! this->checkValue(i))
                return false;
            i += 1;
            if (this->args[i] == string("h")) {
                this->category = ArgParse::HIKING;
                continue;
            }
            if (this->args[i] == string("mc")) {
                this->category = ArgParse::MOUNTAIN_CLIMBING;
                continue;
            }
            if (this->args[i] == string("p")) {
                this->category = ArgParse::PARADE;
                continue;
            }
            if (this->args[i] == string("s")) {
                this->category = ArgParse::SOCCER;
                continue;
            }
            this->log->error(string("Invalid category '") + this->args[i]
                   + string("'. Run with -h to show available categories."));
            return false;
        }

        // Smooth
        if ((this->args[i] == string("-s")) || (this->args[i] == string("--smooth"))) {
            this->smooth = true;
            continue;
        }

        // OpenMP threads
        if ((this->args[i] == string("-t")) || (this->args[i] == string("--threads"))) {
            if (! this->checkValue(i))
                return false;
            i += 1;
            #ifdef _OPENMP
                omp_set_num_threads(stoi(this->args[i]));
            #endif
            continue;
        }

        // Skip steps
        if (this->args[i] == string("--skip")) {
            if (! this->checkValue(i))
                return false;
            i += 1;
            if (this->args[i] == string("g")) {
                this->skip |= ArgParse::SKIP_GLIMPSES;
                continue;
            }
            if (this->args[i] == string("s")) {
                this->skip |= ArgParse::SKIP_SALIENCY;
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

    // Check if category was chosen if using C3D
    if ((this->method == ArgParse::GLIMPSES) && (this->submethod == ArgParse::GLIMPSES_C3D)
                                             && (this->category == ArgParse::UNASSIGNED)) {
        this->log->error("Parameter -c is required for gC3D method.");
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

bool ArgParse::checkValue(int idx) {
    if (this->args.size() < idx + 2) {
        this->log->error(string("Argument ") + this->args[idx] + string(" needs a value."));
        return false;
    }

    return true;
}
