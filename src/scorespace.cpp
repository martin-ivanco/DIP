#include "scorespace.hpp"

using namespace std;

ScoreSpace::ScoreSpace(int splitCount, int splitLength) {
    this->space = vector<vector<vector<double>>>(splitCount,
        vector<vector<double>>(LENGTH(Glimpses::PHIS), vector<double>(LENGTH(Glimpses::LAMBDAS))));
    this->accumulator = vector<vector<vector<Trace>>>(splitCount,
        vector<vector<Trace>>(LENGTH(Glimpses::PHIS), vector<Trace>(LENGTH(Glimpses::LAMBDAS))));
}

void ScoreSpace::set(int time, int phi, int lambda, double score) {
    int phiIndex = 0;
    if (phi == Glimpses::PHIS[1]) phiIndex = 1;
    if (phi == Glimpses::PHIS[2]) phiIndex = 2;
    if (phi == Glimpses::PHIS[3]) phiIndex = 3;
    if (phi == Glimpses::PHIS[4]) phiIndex = 4;
    if (phi == Glimpses::PHIS[5]) phiIndex = 5;
    if (phi == Glimpses::PHIS[6]) phiIndex = 6;
    if (phi == Glimpses::PHIS[7]) phiIndex = 7;
    if (phi == Glimpses::PHIS[8]) phiIndex = 8;
    if (phi == Glimpses::PHIS[9]) phiIndex = 9;
    if (phi == Glimpses::PHIS[10]) phiIndex = 10;
    int lambdaIndex = (lambda + 180) / 20;
    this->space[time][phiIndex][lambdaIndex] = score;
}

bool ScoreSpace::findTrajectory(Trajectory &trajectory) {    
    // Initialization - scores from first time split
    for (int p = 0; p < LENGTH(Glimpses::PHIS); p++) {
        for (int l = 0; l < LENGTH(Glimpses::LAMBDAS); l++) {
            this->accumulator[0][p][l] = Trace {-1, -1, this->space[0][p][l]};
        }
    }

    // Main section - find best ancestor and add score
    for (int t = 1; t < this->space.size(); t++) {
        for (int p = 0; p < LENGTH(Glimpses::PHIS); p++) {
            for (int l = 0; l < LENGTH(Glimpses::LAMBDAS); l++) {
                Trace a = this->findBestAncestor(t, p, l);
                this->accumulator[t][p][l] = 
                    Trace {a.phi, a.lambda, a.score + this->space[t][p][l]};
            }
        }
    }

    // Finding the best score in the final time split
    Trace bestEnd = {-1, -1, 0};
    for (int p = 0; p < LENGTH(Glimpses::PHIS); p++) {
        for (int l = 0; l < LENGTH(Glimpses::LAMBDAS); l++) {
            if (bestEnd.score < this->accumulator[this->space.size() - 1][p][l].score) {
                trajectory[this->space.size() * this->splitLength - ((this->splitLength + 1) / 2)] = tPoint(p, l, ScoreSpace::AOV);
                bestEnd = this->accumulator[this->space.size() - 1][p][l];
            }
        }
    }

    // Backtracking - find the path that leads to the best score
    Trace pTrace = bestEnd;
    for (int t = this->space.size() - 2; t >= 0; t--) {
        trajectory[t * this->splitLength + this->splitLength / 2] = tPoint(pTrace.phi, pTrace.lambda, ScoreSpace::AOV);
        pTrace = this->accumulator[t][pTrace.phi][pTrace.lambda];
    }

    // this->saveToFile(); // for development only
    return trajectory.interpolate(this->splitLength);
}

Trace ScoreSpace::findBestAncestor(int time, int phi, int lambda) {
    Trace t = {-1, -1, 0};

    // Find the best score in previous layer with close phi and lambda
    for (int p = -1; p < 2; p++) {
        if ((phi + p >= 0) && (phi + p < LENGTH(Glimpses::PHIS))) {
            for (int l = -1; l < 2; l++) {
                if ((lambda + l >= 0) && (lambda + l < LENGTH(Glimpses::LAMBDAS))) {
                    if (t.score <= this->accumulator[time - 1][phi + p][lambda + l].score)
                        t = {phi + p, lambda + l,
                            this->accumulator[time - 1][phi + p][lambda + l].score};
                }
            }
        } 
    }

    return t;
}

void ScoreSpace::interpolate(vector<tuple<double, double, double>> &trajectory) {
    // Copying the first point on the trajectory to fill the beginning
    int startIndex = this->splitLength / 2;
    int endIndex;
    for (int i = 0; i < this->splitLength / 2; i++)
        trajectory[i] = trajectory[startIndex];

    // Main loop - linarly interpolating between each neighbouring pair of points
    for (int i = 0; i < this->space.size() - 1; i++) {
        endIndex = startIndex + this->splitLength;
        tuple<double, double, double> s = trajectory[startIndex];
        tuple<double, double, double> e = trajectory[endIndex];
        for (int j = startIndex + 1; j < endIndex; j++) {
            double r = static_cast<double>(j - startIndex) / this->splitLength;
            trajectory[j] = make_tuple((1 - r) * get<0>(s) + r * get<0>(e),
                                       (1 - r) * get<1>(s) + r * get<1>(e),
                                       (1 - r) * get<2>(s) + r * get<2>(e));
        }
        startIndex = endIndex;
    }
    
    // Copying the last point on the trajectory to fill the end
    for (int i = endIndex + 1; i < trajectory.size(); i++)
        trajectory[i] = trajectory[endIndex];
}

// for development only
void ScoreSpace::saveToFile() {
    ofstream file1("../Playground/space1.txt");
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < LENGTH(Glimpses::PHIS); p++) {
            for (int l = 0; l < LENGTH(Glimpses::LAMBDAS); l++) {
                file1 << this->space[s][p][l] << "\t ";
            }
            file1 << "\n";
        }
        file1 << "\n\n\n";
    }
    ofstream file2("../Playground/accumulator.txt");
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < LENGTH(Glimpses::PHIS); p++) {
            for (int l = 0; l < LENGTH(Glimpses::LAMBDAS); l++) {
                file2 << this->accumulator[s][p][l].score << "\t ";
            }
            file2 << "\n";
        }
        file2 << "\n\n\n";
    }
}

// for development only
void ScoreSpace::loadFromFile() {
    ifstream file1("../Playground/space.txt");
    if (! file1.is_open())
        throw runtime_error("Could not open score space file.");

    char temp;
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < LENGTH(Glimpses::PHIS); p++) {
            for (int l = 0; l < LENGTH(Glimpses::LAMBDAS); l++) {
                file1 >> this->space[s][p][l];
            }
        }
    }
}
