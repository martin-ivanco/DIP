#include "trajectory.hpp"

using namespace std;

Trajectory::Trajectory(Logger &log, int length) {
    this->log = &log;
    this->path = vector<tPoint>(length, tPoint(0, 0, 0));
}

bool Trajectory::interpolate(int splitLength) {
    this->log->debug("Interpolating trajectory.");
    int startIndex;
    int endIndex;

    // Copying the first point on the trajectory to fill the beginning
    if (this->path.size() > splitLength) {
        startIndex = splitLength / 2;
        for (int i = 0; i < splitLength / 2; i++)
            this->path[i] = this->path[startIndex];
    }
    else {
        // Video shorter that split size -> stays at single point throughout
        startIndex = this->path.size() / 2;
        for (int i = 0; i < this->path.size(); i++)
            this->path[i] = this->path[startIndex];
        return true;
    }    

    // Main loop - linarly interpolating between each neighbouring pair of points
    for (int i = 0; i < this->path.size() / splitLength - 1; i++) {
        endIndex = startIndex + splitLength;
        for (int j = startIndex + 1; j < endIndex; j++) {
            double r = static_cast<double>(j - startIndex) / splitLength;
            this->path[j] = (this->path[startIndex] * (1 - r)) + (this->path[endIndex] * r);
        }
        startIndex = endIndex;
    }

    // If the size isn't divisible by split length, doing one more iteration
    int modulo = this->path.size() % splitLength;
    if (modulo != 0) {
        endIndex += (splitLength + modulo) / 2;
        for (int j = startIndex + 1; j < endIndex; j++) {
            double r = static_cast<double>(j - startIndex) / splitLength;
            this->path[j] = (this->path[startIndex] * (1 - r)) + (this->path[endIndex] * r);
        }
    }
    
    // Copying the last point on the trajectory to fill the end
    for (int i = endIndex + 1; i < this->path.size(); i++)
        this->path[i] = this->path[endIndex];

    return true;
}

int Trajectory::length() {
    return this->path.size();
}

tPoint &Trajectory::operator[](size_t idx) {
    if (idx < this->path.size())
        return this->path[idx];
    this->log->error("Out of bounds index for trajectory (" + to_string(idx) + " >= "
                     + to_string(this->path.size()) + "). Appending a point and returning it.");
    this->path.push_back(tPoint(0, 0, 0));
    return this->path[this->path.size() - 1];
}

void Trajectory::save(string path) { // development
    this->log->debug("Saving trajectory to file '" + path + "'.");
    ofstream file(path);
    for (int i = 0; i < this->path.size(); i++) {
        file << this->path[i].phi << " " << this->path[i].lambda << " " << this->path[i].aov
             << endl;
    }
}

void Trajectory::load(string path) { // development
    this->log->debug("Loading trajectory from file '" + path + "'.");
    ifstream file(path);
    for (int i = 0; i < this->path.size(); i++) {
        file >> this->path[i].phi >> this->path[i].lambda >> this->path[i].aov;
    }
}
