#include "glimpses.hpp"
#include "saliency.hpp"
#include "scorespace.hpp"

using namespace std;

int main(int argc, char **argv) {
    try {
        Glimpses glimpses = Glimpses(); // for development only
        Saliency saliency = Saliency(glimpses, S_ITTI);
        ScoreSpace space = saliency.getScoreSpace();
        vector<vector<int>> path = space.getShortestPath();
        for (auto i: path)
            cerr << i[0] << " " << i[1] << endl;
    }
    catch (const invalid_argument& a) {
        cerr << "Invalid argument. The file doesn't exist or isn't an mp4." << endl;
        return 1;
    }
    catch (const runtime_error& e) {
        cerr << e.what() << endl;
        return 2;
    }
    
    return 0;
}