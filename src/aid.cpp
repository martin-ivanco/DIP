#include "aid.hpp"

using namespace std;

const string AID::FACE_DETECTOR_PATH =
    "external/openface/lib/local/LandmarkDetector/model/mtcnn_detector/MTCNN_detector.txt";

AID::AID(Logger &log) {
    this->log = &log;
    this->face_detector = LandmarkDetector::FaceDetectorMTCNN(AID::FACE_DETECTOR_PATH);
}

bool AID::findTrajectory(Trajectory &trajectory, VideoInfo input, bool continuous) {
    if (continuous) {
        this->log->error("Continuous automatic importance detection not yet implemented.");
        return false;
    }

    // Opening input video
    cv::VideoCapture video = cv::VideoCapture(input.path);
    if (! video.isOpened()) {
        this->log->error("Couldn't open video.");
        return false;
    }
    int ri = AID::RECALCULATE_INTERVAL * input.fps;

    // Temporary variables
    cv::Mat prev_frame, curr_frame;
    video.read(curr_frame);
    cv::Mat saliency_map, optical_flow, face_detection;
    vector<cv::Point2f> prev_points, curr_points;
    cv::Mat temp_flow;
    int shot_start_index = 0;

    // Main evaluation loop
    for (int i = 0; ! curr_frame.empty(); i++) {
        this->log->debug("Evaluating frame " + to_string(i) + ".");

        // Recalculate important points when interval is reached
        if (i % ri == 0) {
            prev_points.clear();
            curr_points = this->getImportantPoints(curr_frame);
        }

        // Calculate optical flow first to check if this is a new shot
        temp_flow = this->getOpticalFlow(prev_frame, curr_frame, prev_points, curr_points, true);
        if (this->checkNewShot(prev_points, curr_points, input.size)) {
            // If it is, count coordinates and add them to trajectory
            tPoint coords = this->getCoords(saliency_map, optical_flow, face_detection);
            for (int j = shot_start_index; j < i; j++)
                trajectory[j] = coords;
            
            // Clear feature maps and other stuff
            saliency_map = cv::Mat();
            optical_flow = cv::Mat();
            face_detection = cv::Mat();
            prev_points.clear();
            curr_points = this->getImportantPoints(curr_frame);
            temp_flow = cv::Mat::zeros(temp_flow.rows, temp_flow.cols, temp_flow.type());
        }

        // Calculate saliency map and face detection
        saliency_map.push_back(this->getSaliencyMap(curr_frame, true));
        optical_flow.push_back(temp_flow);
        face_detection.push_back(this->getFaceDetection(curr_frame, true));
        
        // Move current data to previous and get new frame
        prev_frame = curr_frame.clone();
        prev_points = curr_points;
        video.read(curr_frame);
    }

    return true;
}

vector<cv::Point2f> AID::getImportantPoints(cv::Mat &frame) {
    // Convert frame to grayscale
    cv::Mat grayscale;
    cv::cvtColor(frame, grayscale, cv::COLOR_BGR2GRAY);

    // Get important points
    vector<cv::Point2f> points;
    cv::goodFeaturesToTrack(grayscale, points, AID::MAX_TRACKING_POINTS, AID::MIN_POINT_QUALITY,
                            AID::MIN_POINT_DISTANCE, cv::Mat(), 3, true);
    return points;
}

cv::Mat AID::getSaliencyMap(cv::Mat &frame, bool compact) {
    // Resize frame to MBS size and convert to Lab color space
    cv::Mat resized;
    cv::Size target_size(AID::MBS_SIZE, (int) (AID::MBS_SIZE * frame.rows / frame.cols));
    cv::resize(frame, resized, target_size, 0, 0, cv::INTER_AREA);
    cv::cvtColor(resized, resized, cv::COLOR_RGB2Lab);
    
    // Compute saliency
    MBS mbs(resized);
    mbs.computeSaliency();
    cv::Mat saliency = mbs.getSaliencyMap();

    // Resize back to original size and compact the feature if requested
    cv::Mat feature;
    cv::resize(saliency, feature, frame.size());
    if (compact) {
        cv::Mat row_feature;
        cv::reduce(feature, row_feature, 0, cv::REDUCE_SUM, CV_32FC1);
        feature = row_feature;
    }

    // Return normalized feature
    cv::Mat norm_feature;
    cv::normalize(feature, norm_feature, 0.0, 1.0, cv::NORM_MINMAX, CV_32FC1);
    return norm_feature;
}

cv::Mat AID::getOpticalFlow(cv::Mat &prev_frame, cv::Mat &curr_frame,
                            vector<cv::Point2f> &prev_points, vector<cv::Point2f> &curr_points,
                            bool compact) {
    // If previous points are missing, return zeros
    if (prev_points.empty())
        return cv::Mat::zeros(1, curr_frame.cols, CV_32FC1);
    
    // Convert frames to grayscale
    cv::Mat prev_gray, curr_gray;
    cv::cvtColor(prev_frame, prev_gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(curr_frame, curr_gray, cv::COLOR_BGR2GRAY);

    // Calculate optical flow
    vector<uchar> status;
    vector<float> err;
    curr_points.clear();
    cv::calcOpticalFlowPyrLK(prev_gray, curr_gray, prev_points, curr_points, status, err);

    // Generate feature map
    cv::Mat feature(curr_frame.rows, curr_frame.cols, CV_32FC1, cv::Scalar(0));
    vector<cv::Point2f> good_ones;
    for (int i = 0; i < curr_points.size(); i++) {
        // Check if corresponding point has been found
        if (status[i] != 1)
            continue;
        
        // If a corresponding previous point exists,
        // calculate distance travelled and set it to feature
        if (i < prev_points.size()) {
            cv::Point2f diff = prev_points[i] - curr_points[i];
            float dist = cv::sqrt(diff.x * diff.x + diff.y * diff.y);
            // TODO maybe smooth falloff
            cv::rectangle(feature, this->getAreaRect(curr_points[i], feature), cv::Scalar(dist),
                          cv::FILLED);
        }

        // Append the point to good ones
        good_ones.push_back(curr_points[i]);
    }

    // Just take the successfully found points and compact the feature if requested
    curr_points = good_ones;
    if (compact) {
        cv::Mat row_feature;
        cv::reduce(feature, row_feature, 0, cv::REDUCE_SUM, CV_32FC1);
        feature = row_feature;
    }

    // Return normalized feature
    cv::Mat norm_feature;
    cv::normalize(feature, norm_feature, 1, 0, cv::NORM_MINMAX, CV_32FC1);
    return norm_feature;
}

cv::Mat AID::getFaceDetection(cv::Mat &frame, bool compact) {
    // Detect faces
    vector<cv::Rect2f> faces;
    vector<float> confidences;
    LandmarkDetector::DetectFacesMTCNN(faces, frame, this->face_detector, confidences);

    // Generate feature map
    cv::Mat feature(frame.rows, frame.cols, CV_32FC1, cv::Scalar(0));
    for(auto f : faces) // TODO maybe smooth falloff and confidence
        cv::rectangle(feature, f, cv::Scalar(1), cv::FILLED); 

    // Compact the feature if requested
    if (compact) {
        cv::Mat row_feature;
        cv::reduce(feature, row_feature, 0, cv::REDUCE_SUM, CV_32FC1);
        feature = row_feature;
    }

    // Return normalized feature
    cv::Mat norm_feature;
    cv::normalize(feature, norm_feature, 1, 0, cv::NORM_MINMAX, CV_32FC1);
    return norm_feature;
}

bool AID::checkNewShot(vector<cv::Point2f> &prev_points, vector<cv::Point2f> &curr_points,
                       cv::Size frame_size) {
    // If it's the first frame of an already new shot, prev_points will be empty
    if (prev_points.empty())
        return false;

    // If we lost more than quarter of the points, it's definitely a new shot
    if (curr_points.size() * 4 < prev_points.size() * 3)
        return true;

    // Also, if at least quarter of the points moved more than quarter of the image
    float threshold = 0.25 * cv::sqrt(frame_size.width * frame_size.width
                                      + frame_size.height * frame_size.height);
    int count = curr_points.size() / 4;
    for (int i = 0; i < curr_points.size(); i++) {
        cv::Point2f diff = prev_points[i] - curr_points[i];
        if (cv::sqrt(diff.x * diff.x + diff.y * diff.y) > threshold)
            count--;
        if (count < 0)
            break;
    }
    return count < 0;
}

tPoint AID::getCoords(cv::Mat &saliency_map, cv::Mat &optical_flow, cv::Mat &face_detection) {
    return tPoint(0, 0, 0); // TODO
}

cv::Rect2f AID::getAreaRect(cv::Point2f &point, cv::Mat &mat, float radius) {
    float xMin = point.x - radius < 0 ? 0 : point.x - radius;
    float xMax = point.x + radius > mat.cols - 1 ? mat.cols - 1 : point.x + radius;
    float yMin = point.y - radius < 0 ? 0 : point.y - radius;
    float yMax = point.y + radius > mat.rows - 1 ? mat.rows - 1 : point.y + radius;
    return cv::Rect2f(xMin, yMin, xMax - xMin, yMax - yMin);
}
