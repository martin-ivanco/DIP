#ifndef __TRAJECTORY__
#define __TRAJECTORY__

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include "logger.hpp"

using namespace std;

struct tPoint {
    double phi;
    double lambda;
    double aov;

    tPoint(double phi, double lambda, double aov) {
        this->phi = phi;
        this->lambda = lambda;
        this->aov = aov;
    }

    void operator=(tPoint p) {
        this->phi = p.phi;
        this->lambda = p.lambda;
        this->aov = p.aov;
    }

    void operator+=(tPoint p) {
        this->phi += p.phi;
        this->lambda += p.lambda;
        this->aov += p.aov;
    }

    tPoint operator+(tPoint p) {
        return tPoint(this->phi + p.phi, this->lambda + p.lambda, this->aov + p.aov);
    }

    tPoint operator*(double n) {
        return tPoint(this->phi * n, this->lambda * n, this->aov * n);
    }

    tPoint operator/(double n) {
        return tPoint(this->phi / n, this->lambda / n, this->aov / n);
    }
};

class Trajectory {

private:
    Logger *log;
    vector<tPoint> path;

    tPoint between(tPoint &start, tPoint &end, double ratio);

public:
    Trajectory(Logger &log, int length);
    bool interpolate(int splitLength);
    bool smooth(int splitLenght);
    int length();
    tPoint &operator[](size_t idx);

    void save(string path); // development
    void load(string path); // development

};

#endif // __TRAJECTORY__