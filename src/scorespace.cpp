#include "scorespace.hpp"

using namespace std;

ScoreSpace::ScoreSpace(int splitCount) {
    this->space = vector<vector<vector<double>>>(splitCount, vector<vector<double>>(PHIS_LENGTH, vector<double>(LAMBDAS_LENGTH)));
    this->accumulator = vector<vector<vector<Trace>>>(splitCount, vector<vector<Trace>>(PHIS_LENGTH, vector<Trace>(LAMBDAS_LENGTH)));
}

void ScoreSpace::set(int time, int phi, int lambda, double score) {
    int phiIndex = 0;
    if (phi == PHIS[1]) phiIndex = 1;
    if (phi == PHIS[2]) phiIndex = 2;
    if (phi == PHIS[3]) phiIndex = 3;
    if (phi == PHIS[4]) phiIndex = 4;
    if (phi == PHIS[5]) phiIndex = 5;
    if (phi == PHIS[6]) phiIndex = 6;
    if (phi == PHIS[7]) phiIndex = 7;
    if (phi == PHIS[8]) phiIndex = 8;
    if (phi == PHIS[9]) phiIndex = 9;
    if (phi == PHIS[10]) phiIndex = 10;
    int lambdaIndex = (lambda + 180) / 20;
    this->space[time][phiIndex][lambdaIndex] = score;
}

vector<vector<int>> ScoreSpace::getShortestPath() {
    vector<vector<int>> path(this->space.size(), vector<int>(2));
    
    for (int p = 0; p < PHIS_LENGTH; p++) {
        for (int l = 0; l < LAMBDAS_LENGTH; l++) {
            this->accumulator[0][p][l] = Trace {-1, -1, this->space[0][p][l]};
        }
    }

    for (int t = 1; t < this->space.size(); t++) {
        for (int p = 0; p < PHIS_LENGTH; p++) {
            for (int l = 0; l < LAMBDAS_LENGTH; l++) {
                Trace a = this->findBestAncestor(t, p, l);
                this->accumulator[t][p][l] = Trace {a.phi, a.lambda, a.score + this->space[t][p][l]};
            }
        }
    }

    Trace bestEnd = {-1, -1, 0};
    for (int p = 0; p < PHIS_LENGTH; p++) {
        for (int l = 0; l < LAMBDAS_LENGTH; l++) {
            if (bestEnd.score < this->accumulator[this->space.size() - 1][p][l].score) {
                path[this->space.size() - 1][0] = p;
                path[this->space.size() - 1][1] = l;
                bestEnd = this->accumulator[this->space.size() - 1][p][l];
            }
        }
    }

    Trace pTrace = bestEnd;
    for (int t = this->space.size() - 2; t >= 0; t--) {
        path[t][0] = pTrace.phi;
        path[t][1] = pTrace.lambda;
        pTrace = this->accumulator[t][pTrace.phi][pTrace.lambda];
    }

    this->saveToFile();
    return path;
}

Trace ScoreSpace::findBestAncestor(int time, int phi, int lambda) {
    Trace t = {-1, -1, 0};
    for (int p = -1; p < 2; p++) {
        if ((phi + p >= 0) && (phi + p < PHIS_LENGTH)) {
            for (int l = -1; l < 2; l++) {
                if ((lambda + l >= 0) && (lambda + l < LAMBDAS_LENGTH)) {
                    if (t.score <= this->accumulator[time - 1][phi + p][lambda + l].score)
                        t = {phi + p, lambda + l, this->accumulator[time - 1][phi + p][lambda + l].score};
                }
            }
        } 
    }

    return t;
}

void ScoreSpace::saveToFile() {
    ofstream file1("../Playground/space.txt");
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < PHIS_LENGTH; p++) {
            for (int l = 0; l < LAMBDAS_LENGTH; l++) {
                file1 << this->space[s][p][l] << "\t ";
            }
            file1 << "\n";
        }
        file1 << "\n\n\n";
    }
    ofstream file2("../Playground/accumulator.txt");
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < PHIS_LENGTH; p++) {
            for (int l = 0; l < LAMBDAS_LENGTH; l++) {
                file2 << this->accumulator[s][p][l].score << "\t ";
            }
            file2 << "\n";
        }
        file2 << "\n\n\n";
    }
}
