#include "glimpses.hpp"

using namespace std;

int main(int argc, char **argv) {
    try {
        Glimpses glimpses = Glimpses("../Playground/test.mp4");
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