#ifndef BIRL_TUNING_H
#define BIRL_TUNING_H

#include "Birl.h"

typedef enum {EQUAL_TEMPERED, JUST_INTONATION, MEANTONE, HIGHLAND_BAGPIPE, CUSTOM} tuningSystem;
enum {NUM_NOTES = 11};
double justIntonation[] = {10.0/4.0, 18.0/8.0, 2.0/1.0, 15.0/8.0, 5.0/3.0, 3.0/2.0, 4.0/3.0, 5.0/4.0, 9.0/8.0, 1.0, 15.0/16.0};
/* double justIntonation[] = {10.0/4.0, 18.0/8.0, 2.0/1.0, 15.0/8.0, 5.0/3.0, 3.0/2.0, 4.0/3.0, 5.0/4.0, 9.0/8.0, 1.0, 15.0/16.0, 5.0/6.0, 3.0/4.0, 4.0/6.0, 5.0/8.0}; */
double equalTempered[] = {2.519840, 2.244920, 2.000000, 1.887750, 1.681790, 1.498310, 1.334830, 1.259920, 1.122460, 1.000000, 0.9439};
double meanTone[] = {2.5, 2.236, 2.0000, 1.8692, 1.6719, 1.4953, 1.3375, 1.2500, 1.1180, 1.0000, 0.9346};
double highlandBagpipe[] = {2.25, 2., 1.8, 1.66666, 1.5, 1.35, 1.25, 1.125, 1, 0.9, 0.83333};
double customTuning[] = {2.519840, 2.244920, 2.000000, 1.887750, 1.681790, 1.498310, 1.334830, 1.259920, 1.122460, 1.000000, 0.9439};

int FUNDAMENTAL_INDEX = NUM_NOTES - 2;
const double MIN_D1 = 1.0; // = 0.002 * 2 * 200
const double DH_FIRST_GUESS = 1.0;
const double CM_PER_INCH = 2.54;

static double *tuning;

static double convertToSamples(double cm) {
    return (cm * ((SRATE*OVERSAMPLE) / C_cm));
}

static double convertTocm(double samps) {
    return (samps * (C_cm / (SRATE*OVERSAMPLE)));
}

static double calcg(int thNum) {
    if (thNum < 0 || thNum > NUM_NOTES-2) {
        printf("thNum out of bounds: %d\n", thNum);
        return 0.0;
    }
    return (tuning[thNum] / tuning[thNum+1]) - 1.0;
}

static double calcd1(int LC, double LS) {
    return (LS - (double) LC)/(0.3 * convertToSamples(CM_PER_INCH));
    /* return (LS - (double) LC)/0.3; */
}

// In samples.
static int calcLC(double LS) {
    double LC = (int) LS;
    double d1 = calcd1(LC, LS);
    while (d1 < MIN_D1) {
        printf("d1 = %f for this value of LC so we're shortening LC!!!!!!\n", d1);
        LC -= 1.0;
        d1 = calcd1(LC, LS);
    }
    return LC;
}

// In samples.
static double calcLS(double Fc) {
    return (SRATE*OVERSAMPLE)/(4.0 * Fc);
}

static int calclL(double d1, int thNum, double LS) {
    double dH = 1.0 * OVERSAMPLE;
    double g = calcg(thNum);
    double LSh = (1.0/ tuning[thNum]) * LS;
    double LBh = dH * ((d1*d1)/(dH*dH)) - 0.45*d1;
    double z = 0.5 * g * sqrt(1 + 4*(LBh/(g*LSh))) - 0.5*g;
    return (int) (LSh - (z*LSh));
}

// In samples iff LSh is in samples.
static double calcLBh(int thNum, double LSh, int lLint) {
    double g = calcg(thNum);
    double lL = (double) lLint;

    double gLSh = g * LSh;
    double nmrtr1 = (LSh + 0.5*gLSh - lL) * (LSh + 0.5*gLSh - lL);
    nmrtr1 /= gLSh;
    return nmrtr1 - (gLSh / 4.0);
}

static double calcdH(int thNum, double d1, double LS, int lL) {
    double LSh = (1.0/tuning[thNum]) * LS;
    double LBh = calcLBh(thNum, LSh, lL);
    return (d1*d1) / (LBh + 0.45 * d1);
}

static void populateCustomTuning(double freqs[NUM_NOTES]) {
    /* double Fc = freqs[FUNDAMENTAL_INDEX]; */
    double Fc = freqs[NUM_NOTES - 1];
    /* printf("fundamental: %f\n", Fc); */
    for (int i = 0; i < NUM_NOTES; i++) {
        customTuning[i] = freqs[i] / Fc;
    }
}

static void setTuning(tuningSystem t) {
    switch (t) {
        case JUST_INTONATION: {
            tuning = justIntonation;
            break;
        }
        case EQUAL_TEMPERED: {
            tuning = equalTempered;
            break;
        }
        case MEANTONE: {
            tuning = meanTone;
            break;
        }
        case HIGHLAND_BAGPIPE: {
            tuning = highlandBagpipe;
            break;
        }
        case CUSTOM: {
            tuning = customTuning;
            break;
        }
        default: {
            break;
        }
    }
}

static double checkTuning(double d1, double dH, double LSh, double lL, double g) {
    double LBh = dH * ((d1*d1)/(dH*dH)) - 0.45*d1;
    double z = 0.5 * g * sqrt(1 + 4*(LBh/(g*LSh))) - 0.5*g;
    return (SRATE*OVERSAMPLE)/(4 * (lL + (z*LSh)));
}

#endif
