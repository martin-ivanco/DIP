#ifndef __TOOLS__
#define __TOOLS__

#include <iostream>

#include "constants.hpp"

using namespace std;

class Tools {

public:
    Tools();
    static void print(string message, int severity);

};

#endif // __TOOLS__