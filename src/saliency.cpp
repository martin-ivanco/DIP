#include "saliency.hpp"

using namespace std;

Saliency::Saliency(Glimpses glimpses) : glimpses(glimpses) {

}

void Saliency::getScoreSpace(int saliencyType) {
    VideoInfo glimpseInfo = this->glimpses.get(0);
    cv::VideoCapture glimpse = cv::VideoCapture(glimpseInfo.path);
    if (! glimpse.isOpened())
        return;

    cv::Mat frame;
    glimpse.read(frame);

    if (saliencyType == S_ITTI) {
        SalMapItti itti = SalMapItti(frame);
        cv::imshow("itti", itti.salMap);
        cv::waitKey();
    }
    if (saliencyType == S_MARGOLIN) {
        SalMapMargolin margolin = SalMapMargolin(frame);
        cv::imshow("margolin", margolin.salMap);
        cv::waitKey();
    }
    if (saliencyType == S_STENTIFORD) {
        SalMapStentiford stentiford = SalMapStentiford(frame);
        stentiford.generateSalMap();
        cv::imshow("stentiford", stentiford.salMap);
        cv::waitKey();
    }
}