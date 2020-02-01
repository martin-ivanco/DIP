#ifndef __SCORESPACE__
#define __SCORESPACE__

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <tuple>

#include "glimpses.hpp"
#include "trajectory.hpp"

using namespace std;

struct Trace {
    int phi;
    int lambda;
    int aov;
    double score;

    Trace() {
        this->phi = -1;
        this->lambda = -1;
        this->aov = -1;
        this->score = 0;
    }

    Trace(int phi, int lambda, int aov, double score) {
        this->phi = phi;
        this->lambda = lambda;
        this->aov = aov;
        this->score = score;
    }
};

class ScoreSpace {

private:
    Logger *log;
    vector<vector<vector<vector<double>>>> space;
    vector<vector<vector<vector<Trace>>>> accumulator;

    Trace findBestAncestor(int time, int phi, int lambda, int aov, double epsilon);

public:
    ScoreSpace(int splitCount, Logger &log);
    void set(int time, int phi, int lambda, double aov, double score);
    bool findTrajectory(Trajectory &trajectory, int splitLength, double angleEps);
    
    void save(string path); // for development only
    void load(string path); // for development only
};

#endif // __SCORESPACE__