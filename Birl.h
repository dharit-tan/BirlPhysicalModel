#ifndef BIRL_H
#define BIRL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const int FRONT_TUBES = 1;
static const int MAX_TONEHOLES = 10;
static const int MAX_TUBES = MAX_TONEHOLES + FRONT_TUBES;
static const int SAMPLE_INDEX = 0;
static const int MAX_TUBE_LENGTH = 100;
static const double MIN_TONEHOLE_RADIUS = 0.0001;
static const double MAX_TONEHOLE_RADIUS = 0.004;
static const double RB_TWEAK_FACTOR = 0.0001;
static const double RTH_TWEAK_FACTOR = 0.005;
static double originalRth_[MAX_TONEHOLES];
static double rth_[MAX_TONEHOLES];
static int tubeLengths_[MAX_TUBES];
static const int OVERSAMPLE = 2;

const double C_cm = 34500.0;
const double C_m = 345.0;
const double SRATE = 44100.0;
const double reedTableOffset = 0.7;
const double reedTableSlope = -0.3;
/* const double MIN_D1 = 0.05; */

#endif
