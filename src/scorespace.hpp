#ifndef __SCORESPACE__
#define __SCORESPACE__

#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>

#include "glimpses.hpp"

using namespace std;

struct Trace {
    int phi;
    int lambda;
    double score;
};

// TODO refactor
class ScoreSpace {

private:
    static constexpr double AOV = 104.3; // for development only

    vector<vector<vector<double>>> space;
    vector<vector<vector<Trace>>> accumulator;
    int splitLength; // in frames

    Trace findBestAncestor(int time, int phi, int lambda);
    void interpolate(vector<tuple<double, double, double>> &trajectory);
    void saveToFile(); // for development only

public:
    ScoreSpace(int splitCount, int splitLength);
    void set(int time, int phi, int lambda, double score);
    vector<tuple<double, double, double>> getBestTrajectory();
    
    void loadFromFile(); // for development only
};

#endif // __SCORESPACE__