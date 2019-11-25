#include "renderer.hpp"
#include "glimpses.hpp"
#include "saliency.hpp"
#include "scorespace.hpp"

using namespace std;

int main(int argc, char **argv) {
    try {
        Renderer renderer = Renderer("../Playground/test.mp4");
        Glimpses glimpses = Glimpses(renderer);
        Saliency saliency = Saliency(glimpses, S_ITTI);
        ScoreSpace space = saliency.getScoreSpace();
        vector<tuple<int, int>> path = space.getBestPath();
        int p, l;
        for (auto i: path)
            cerr << get<0>(i) << " " << get<1>(i) << endl;
        // renderer.renderPath(space.getBestPath());
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