#ifndef __CONSTANTS__
#define __CONSTANTS__

static const bool DEVELOPMENT = true;

static const int SPLIT_LENGTH_SECONDS = 5;

static const int PHIS_LENGTH = 11;
static const int LAMBDAS_LENGTH = 18;

static const int PHIS[PHIS_LENGTH] = {-75,-45,-30, -20, -10, 0, 10, 20, 30, 45, 75};
static const int LAMBDAS[LAMBDAS_LENGTH] = {-180, -160, -140, -120, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 120, 140, 160};
static const int GLIMPSE_WIDTH = 640;
static const int GLIMPSE_HEIGHT = 360;

static const int S_ITTI = 0;
static const int S_MARGOLIN = 1;
static const int S_STENTIFORD = 2;

#endif // __CONSTANTS__