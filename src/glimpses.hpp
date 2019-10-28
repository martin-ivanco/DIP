#ifndef __GLIMPSES__
#define __GLIMPSES__

#include <iostream>
#include <filesystem>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

class Glimpses {
    
private:
    string originalPath;
    string name;
    string folderPath;

    void generateGlimpses();
    void splitVideo();
    void composeView();
    string getGlimpseName(int timeBlock, int longitude, int latitude);
    void bilinear();

public:
    Glimpses(string videoFilePath);
    
};

#endif // __GLIMPSES__