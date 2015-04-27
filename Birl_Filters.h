#ifndef BIRL_FILTERS_H
#define BIRL_FILTERS_H

#include "Birl.h"

typedef struct _DCFilter {
    double lastInput;
    double lastOutput;
    double gain;
} DCFilter;

static DCFilter *initDCFilter(double gain) {
    DCFilter *dc = (DCFilter *) calloc(1, sizeof(DCFilter));
    dc->lastInput = 0.0;
    dc->lastOutput = 0.0;
    dc->gain = gain;
    return dc;
}

static double inputDCFilter(DCFilter *dc, double input) {
    double output = input - dc->lastInput + (dc->gain * dc->lastOutput);
    dc->lastInput = input;
    dc->lastOutput = output;
    return output;
}

typedef struct _Biquad {
    double a0, a1, a2, b1, b2;
    double z1, z2;
} Biquad;

static Biquad *initBiquad() {
    Biquad *biquad = (Biquad *) calloc(1, sizeof(Biquad));
    biquad->a0 = 1.0;
    biquad->a1 = biquad->a2 = biquad->b1 = biquad->b2 = 0.0;
    biquad->z1 = biquad->z2 = 0.0;
    return biquad;
}

static void biquadSetCoeffs(Biquad *biquad, double a0, double a1, double a2, double b1, double b2) {
    biquad->a0 = a0;
    biquad->a1 = a1;
    biquad->a2 = a2;
    biquad->b1 = b1;
    biquad->b2 = b2;
}

static double inputBiquad(Biquad *biquad, double input) {
    double out = input * biquad->a0 + biquad->z1;
    biquad->z1 = input * biquad->a1 + biquad->z2 - biquad->b1 * out;
    biquad->z2 = input * biquad->a2 - biquad->b2 * out;
    return out;
}

typedef struct _PoleZero {
    double b0, b1, a1;
    double x1, x2;
    double z1, z2;
} PoleZero;

static PoleZero *initPoleZero() {
    PoleZero *pz = (PoleZero *) calloc(1, sizeof(PoleZero));
    pz->b0 = 1.0;
    pz->b1 = 0.0;
    pz->a1 = 0.0;
    pz->x1 = 0.0;
    pz->x2 = 0.0;
    pz->z1 = 0.0;
}

static void setCoeffsPoleZero(PoleZero *pz, double b0, double b1, double a1) {
    pz->b0 = b0;
    pz->b1 = b1;
    pz->a1 = a1;
}

static void setA1PoleZero(PoleZero *pz, double a1) {
    pz->a1 = a1;
}

static void setB0PoleZero(PoleZero *pz, double b0) {
    pz->b0 = b0;
}

static double inputPoleZero(PoleZero *pz, double input) {
    pz->x1 = input;
    double output = pz->b0 * pz->x1 + pz->b1 * pz->x2 - pz->a1 * pz->z1;
    pz->x2 = pz->x1;
    pz->z1 = output;
    return output;
}

static double z1PoleZero(PoleZero *pz) {
    return pz->z1;
}

typedef struct _SVF {
    double g;
    double k;
    double a1, a2, a3;
    double ic1eq, ic2eq;
} SVF;

static SVF *initSVF(double cutoff, double Q) {
    SVF *svf = (SVF *)calloc(1, sizeof(SVF));
    svf->g = tan((M_PI * cutoff)/(SRATE*OVERSAMPLE));
    svf->k = 1.0/Q;
    svf->a1 = 1/(1 + svf->g*(svf->g + svf->k));
    svf->a2 = svf->g*svf->a1;
    svf->a3 = svf->g*svf->a2;
    svf->ic1eq = 0.0;
    svf->ic2eq = 0.0;
    return svf;
}

static void setCutoffSVF(SVF *svf, double cutoff) {
    svf->g = tan((M_PI * cutoff)/(SRATE*OVERSAMPLE));
    /* printf("setting cutoff with g: %f\n", svf->g); */
    svf->a1 = 1/(1 + svf->g*(svf->g + svf->k));
    svf->a2 = svf->g*svf->a1;
    svf->a3 = svf->g*svf->a2;
}

static void setQSVF(SVF *svf, double Q) {
    svf->k = 1.0/Q;
    svf->a1 = 1/(1 + svf->g*(svf->g + svf->k));
    svf->a2 = svf->g*svf->a1;
    svf->a3 = svf->g*svf->a2;
    /* printf("new a1: %f\n", svf->a1); */
}

static double inputSVFPeak(SVF *svf, double input) {
    double v3 = input - svf->ic2eq;
    double v1 = svf->a1*svf->ic1eq + svf->a2*v3;
    double v2 = svf->ic2eq + svf->a2*svf->ic1eq + svf->a3*v3;
    svf->ic1eq = 2*v1 - svf->ic1eq;
    svf->ic2eq = 2*v2 - svf->ic2eq;
    return input - svf->k*v1 - 2*v2;
}

static double inputSVFNotch(SVF *svf, double input) {
   double v1 = svf->a1*svf->ic1eq + svf->a2*(input - svf->ic2eq);
   double v2 = svf->ic2eq + svf->g*v1;
   svf->ic1eq = 2*v1 - svf->ic1eq;
   svf->ic2eq = 2*v2 - svf->ic2eq;
   return input - svf->k * v1;
}

static double inputSVFLP(SVF *svf, double input) {
   double v1 = svf->a1*svf->ic1eq + svf->a2*(input - svf->ic2eq);
   double v2 = svf->ic2eq + svf->g*v1;
   svf->ic1eq = 2*v1 - svf->ic1eq;
   svf->ic2eq = 2*v2 - svf->ic2eq;
   return v2;
}

static double inputSVFBand(SVF *svf, double input) {
   double v1 = svf->a1*svf->ic1eq + svf->a2*(input - svf->ic2eq);
   double v2 = svf->ic2eq + svf->g*v1;
   svf->ic1eq = 2*v1 - svf->ic1eq;
   svf->ic2eq = 2*v2 - svf->ic2eq;
   return v1;
}

static double inputSVFHP(SVF *svf, double input) {
   double v1 = svf->a1*svf->ic1eq + svf->a2*(input - svf->ic2eq);
   double v2 = svf->ic2eq + svf->g*v1;
   svf->ic1eq = 2*v1 - svf->ic1eq;
   svf->ic2eq = 2*v2 - svf->ic2eq;
   return input - svf->k*v1 - v2;
}

static double tanhClip(double input) {
    return tanh(input);
}

static double clip(double input, double min, double max) {
    if (input < min)
        input = min;
    else if (input > max)
        input = max;
    return input;
}

// shaper
static double shaper(double input, double m_drive) {
    double fx = input * 2.0;    // prescale
    double w, c, xc, xc2, xc4;
    const double sqrt8 = 2.82842712475;
    const double wscale = 1.30612244898; // 1/w(1).

    xc = clip(fx, -sqrt8, sqrt8);
    xc2 = xc*xc;
    c = 0.5*fx*(3. - (xc2));
    xc4 = xc2 * xc2;
    w = (1. - xc2*0.25 + xc4*0.015625) * wscale;
    double shaperOut = w*(c+ 0.05*xc2)*(m_drive + 0.75);
    shaperOut *= 0.5;    // post_scale
    return shaperOut;
}

#endif
