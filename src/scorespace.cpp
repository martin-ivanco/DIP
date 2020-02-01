#include "scorespace.hpp"

using namespace std;

ScoreSpace::ScoreSpace(int splitCount, Logger &log) {
    this->log = &log;
    this->log->debug("Creating score space of size " + to_string(splitCount) + "x"
                     + to_string(Glimpses::PHIS.size()) + "x" + to_string(Glimpses::LAMBDAS.size())
                     + "x" + to_string(Glimpses::AOVS.size()) + ".");
    this->space = vector<vector<vector<vector<double>>>>(
        splitCount, vector<vector<vector<double>>>(
            Glimpses::PHIS.size(), vector<vector<double>>(
                Glimpses::LAMBDAS.size(), vector<double>(
                    Glimpses::AOVS.size()))));
    this->accumulator = vector<vector<vector<vector<Trace>>>>(
        splitCount, vector<vector<vector<Trace>>>(
            Glimpses::PHIS.size(), vector<vector<Trace>>(
                Glimpses::LAMBDAS.size(), vector<Trace>(
                    Glimpses::AOVS.size()))));
    
}

void ScoreSpace::set(int time, int phi, int lambda, double aov, double score) {
    // Find phi index
    int phiIndex = -1;
    for (int i = 0; i < Glimpses::PHIS.size(); i++) {
        if (phi == Glimpses::PHIS[i])
            phiIndex = i;
    }

    // Compute lambda index
    int lambdaIndex = (lambda + 160) / 20;

    // Find aov index
    int aovIndex = -1;
    for (int i = 0; i < Glimpses::AOVS.size(); i++) {
        if (abs(aov - Glimpses::AOVS[i]) < 0.001)
            aovIndex = i;
    }

    this->log->debug("Placing score " + to_string(score) + " at position [" + to_string(time)
                     + ", " + to_string(phiIndex) + ", " + to_string(lambdaIndex) + ", "
                     + to_string(aovIndex) + "].");
    this->space[time][phiIndex][lambdaIndex][aovIndex] = score;
}

bool ScoreSpace::findTrajectory(Trajectory &trajectory, int splitLength, double angleEps) {    
    // Initialization - scores from first time split
    for (int p = 0; p < Glimpses::PHIS.size(); p++) {
        for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
            for (int a = 0; a < Glimpses::AOVS.size(); a++) {
                this->accumulator[0][p][l][a] = Trace(-1, -1, -1, this->space[0][p][l][a]);
            }
        }
    }

    // Main section - find best ancestor and add score
    for (int t = 1; t < this->space.size(); t++) {
        for (int p = 0; p < Glimpses::PHIS.size(); p++) {
            for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
                for (int a = 0; a < Glimpses::AOVS.size(); a++) {
                    Trace trace = this->findBestAncestor(t, p, l, a, angleEps);
                    this->accumulator[t][p][l][a] = Trace(trace.phi, trace.lambda, trace.aov,
                                                          trace.score + this->space[t][p][l][a]);
                }
            }
        }
    }

    // Finding the best score in the final time split
    Trace bestEnd;
    int endIndex = (this->space.size() - 1) * splitLength;
    endIndex += trajectory.length() % splitLength == 0 ? splitLength / 2
                                                       : (trajectory.length() % splitLength) / 2;
    for (int p = 0; p < Glimpses::PHIS.size(); p++) {
        for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
            for (int a = 0; a < Glimpses::AOVS.size(); a++) {
                if (bestEnd.score < this->accumulator[this->space.size() - 1][p][l][a].score) {
                    trajectory[endIndex] = tPoint(Glimpses::PHIS[p], Glimpses::LAMBDAS[l],
                                                  Glimpses::AOVS[a]);
                    bestEnd = this->accumulator[this->space.size() - 1][p][l][a];
                }
            }
        }
    }

    // Backtracking - find the path that leads to the best score
    Trace pTrace = bestEnd;
    for (int t = this->space.size() - 2; t >= 0; t--) {
        trajectory[t * splitLength + splitLength / 2] = tPoint(
            Glimpses::PHIS[pTrace.phi], Glimpses::LAMBDAS[pTrace.lambda], Glimpses::AOVS[pTrace.aov]);
        pTrace = this->accumulator[t][pTrace.phi][pTrace.lambda][pTrace.aov];
    }

    return trajectory.interpolate(splitLength);
}

Trace ScoreSpace::findBestAncestor(int time, int phi, int lambda, int aov, double epsilon) {
    Trace t;

    // Find the best score in previous layer with close phi, lambda and aov
    for (int p = 0; p < Glimpses::PHIS.size(); p++) {
        if (abs(Glimpses::PHIS[phi] - Glimpses::PHIS[p]) > epsilon)
            continue;
        for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
            int diff = abs((Glimpses::LAMBDAS[lambda] - Glimpses::LAMBDAS[l] + 180) % 360) - 180;
            if (abs(diff) > epsilon)
                continue;
            for (int a = 0; a < Glimpses::AOVS.size(); a++) {
                if (t.score < this->accumulator[time - 1][p][l][a].score)
                    t = Trace(p, l, a, this->accumulator[time - 1][p][l][a].score);
            }
        }
    }

    return t;
}

// for development only
void ScoreSpace::save(string path) {
    ofstream file(path);
    file << "SPACE" << "\n";
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < Glimpses::PHIS.size(); p++) {
            for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
                for (int a = 0; a < Glimpses::AOVS.size(); a++)
                    file << this->space[s][p][l][a] << " ";
                file << "\t ";
            }
            file << "\n";
        }
        file << "\n\n\n";
    }

    file << "ACCUMULATOR" << "\n";
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < Glimpses::PHIS.size(); p++) {
            for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
                for (int a = 0; a < Glimpses::AOVS.size(); a++)
                    file << this->accumulator[s][p][l][a].score << " ";
                file << "\t ";
            }
            file << "\n";
        }
        file << "\n\n\n";
    }
}

// for development only
void ScoreSpace::load(string path) {
    ifstream file(path);
    string temp;

    file >> temp; // SPACE
    for (int s = 0; s < this->space.size(); s++) {
        for (int p = 0; p < Glimpses::PHIS.size(); p++) {
            for (int l = 0; l < Glimpses::LAMBDAS.size(); l++) {
                for (int a = 0; a < Glimpses::AOVS.size(); a++)
                    file >> this->space[s][p][l][a];
            }
        }
    }
}
