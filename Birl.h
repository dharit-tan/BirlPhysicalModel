#ifndef BIRL_H
#define BIRL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const int FRONT_TUBES = 1;
const int MAX_TONEHOLES = 9;
const int MAX_TUBES = MAX_TONEHOLES + FRONT_TUBES;
const int SAMPLE_INDEX = 0;
const int MAX_TUBE_LENGTH = 1000;
const double MIN_TONEHOLE_RADIUS = 0.0001;
const double MAX_TONEHOLE_RADIUS = 0.004;
const double RB_TWEAK_FACTOR = 0.0001;
const double RTH_TWEAK_FACTOR = 0.005;
double originalRth_[MAX_TONEHOLES];
double rth_[MAX_TONEHOLES];
int tubeLengths_[MAX_TUBES];
const int OVERSAMPLE = 2;

const double C_cm = 34723.0;
const double C_m = 347.23;
const double SRATE = 44100.0;
const double reedTableOffset = 0.7;
const double reedTableSlope = -0.3;
/* const double MIN_D1 = 0.05; */
const double CM_DIAM_TO_METER_RADIUS = 200.0;
const double REFLECTION_DECAY_FACTOR = -0.995;


#endif
