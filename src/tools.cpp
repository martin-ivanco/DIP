#include "tools.hpp"

using namespace std;

Tools::Tools() {

}

void Tools::print(string message, int severity) {
    if (severity >= PRINT_LEVEL)
        if (severity == L_OUTPUT)
            cout << message << endl;
        else
            cerr << message;
}