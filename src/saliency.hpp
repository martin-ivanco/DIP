#ifndef __SALIENCY__
#define __SALIENCY__

#include <iostream>
#include <string>

#include "glimpses.hpp"
#include "../external/saliency/SalMapItti.h"
#include "../external/saliency/SalMapMargolin.h"
#include "../external/saliency/SalMapStentiford.h"

using namespace std;

static const int S_ITTI = 0;
static const int S_MARGOLIN = 1;
static const int S_STENTIFORD = 2;

class Saliency {

private:
    Glimpses glimpses;

public:
    Saliency(Glimpses glimpses);
    void getScoreSpace(int saliencyType);

};

#endif // __SALIENCY__