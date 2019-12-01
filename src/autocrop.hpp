#ifndef __AUTOCROP__
#define __AUTOCROP__

#include <iostream>
#include <opencv2/core.hpp>

#include "constants.hpp"
#include "../external/saliency/SalMapItti.h"
#include "../external/saliency/SalMapMargolin.h"
#include "../external/saliency/SalMapStentiford.h"
#include "../external/crop/AutocropFang.h"
#include "../external/crop/AutocropStentiford.h"
#include "../external/crop/AutocropSuh.h"

using namespace std;

class AutoCrop {

private:
    vector<tuple<double, double, double>> path;

    cv::Mat getSaliencyMap(cv::Mat frame, int type);
    cv::Rect getROI(cv::Mat frame, cv::Mat saliency, int type);
    tuple<double, double, double> getCoords(cv::Rect roi);
    void saveToFile();
    void loadFromFile(string filePath);

public:
    AutoCrop(string videoFilePath, int saliencyType);
    vector<tuple<double, double, double>> getPath();

};

#endif // __AUTOCROP__