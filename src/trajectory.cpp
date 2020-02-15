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
            this->path[j] = this->between(this->path[startIndex], this->path[endIndex],
                                          static_cast<double>(j - startIndex) / splitLength);
        }
        startIndex = endIndex;
    }

    // If the size isn't divisible by split length, doing one more iteration
    int modulo = this->path.size() % splitLength;
    if (modulo != 0) {
        endIndex = startIndex + (splitLength + modulo) / 2;
        for (int j = startIndex + 1; j < endIndex; j++) {
            this->path[j] = this->between(this->path[startIndex], this->path[endIndex],
                                          static_cast<double>(j - startIndex) / splitLength);
        }
    }
    
    // Copying the last point on the trajectory to fill the end
    for (int i = endIndex + 1; i < this->path.size(); i++)
        this->path[i] = this->path[endIndex];

    return true;
}

bool Trajectory::smooth(int splitLength) {
    this->log->debug("Smoothing trajectory.");

    // Main loop - average coordinates of each split
    for (int i = 0; i < this->path.size() / splitLength; i++) {
        tPoint avg(0, 0, 0);
        for (int j = 0; j < splitLength; j++)
            avg += this->path[i * splitLength + j];
        this->path[i * splitLength + splitLength / 2] = avg / splitLength;
    }

    // If the size isn't divisible by split length, doing one more iteration
    int modulo = this->path.size() % splitLength;
    if (modulo != 0) {
        tPoint avg(0, 0, 0);
        for (int j = 0; j < modulo; j++)
            avg += this->path[this->path.size() - modulo + j];
        this->path[this->path.size() - modulo + modulo / 2] = avg / modulo;
    }

    this->interpolate(splitLength);
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

tPoint Trajectory::between(tPoint &start, tPoint &end, double ratio) {
    // Compute new lambda
    double lambda_diff = abs(fmod(end.lambda - start.lambda + 180, 360)) - 180;
    if ((abs(end.lambda - start.lambda) > 180) && (end.lambda < start.lambda))
        lambda_diff *= -1;
    double new_lambda = start.lambda + lambda_diff * ratio;
    if (new_lambda <= -180) new_lambda += 360;
    if (new_lambda > 180) new_lambda -= 360;
    
    return tPoint(start.phi * (1 - ratio) + end.phi * ratio, new_lambda,
                  start.aov * (1 - ratio) + end.aov * ratio);
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
