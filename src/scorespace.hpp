#ifndef __SCORESPACE__
#define __SCORESPACE__

#define LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#include <iostream>
#include <fstream>
#include <vector>

#include "glimpses.hpp"

using namespace std;

struct Trace {
    int phi;
    int lambda;
    double score;
};

class ScoreSpace {

private:
    vector<vector<vector<double>>> space;
    vector<vector<vector<Trace>>> accumulator;

    Trace findBestAncestor(int time, int phi, int lambda);
    void saveToFile(); // for development only

public:
    ScoreSpace(int splitCount);
    void set(int time, int phi, int lambda, double score);
    vector<tuple<int, int>> getBestPath();
    
    void loadFromFile(); // for development only
};

#endif // __SCORESPACE__