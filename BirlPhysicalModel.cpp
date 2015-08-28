//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chugerate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"

// general includes
#include <stdio.h>
#include <limits.h>

// #include "ReedTable.h"
#include "OneZero.h"
#include "PoleZero.h"
#include "Noise.h"

#include "Birl.h"
#include "Birl_Filters.h"
#include "Birl_Tuning.h"
#include "Birl_Tube.h"

// declaration of chugin constructor
CK_DLL_CTOR(birlphysicalmodel_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(birlphysicalmodel_dtor);

CK_DLL_MFUN(birlphysicalmodel_setBreathPressure);
CK_DLL_MFUN(birlphysicalmodel_setLength);
CK_DLL_MFUN(birlphysicalmodel_setToneHole);
CK_DLL_MFUN(birlphysicalmodel_setToneHoleRadius);
CK_DLL_MFUN(birlphysicalmodel_setFundamental);
CK_DLL_MFUN(birlphysicalmodel_tweakBoreRadius);
CK_DLL_MFUN(birlphysicalmodel_tweakToneHoleRadius);
CK_DLL_MFUN(birlphysicalmodel_lpCutoff);
CK_DLL_MFUN(birlphysicalmodel_lpQ);
CK_DLL_MFUN(birlphysicalmodel_pfCutoff);
CK_DLL_MFUN(birlphysicalmodel_pfQ);
CK_DLL_MFUN(birlphysicalmodel_noiseBPCutoff);
CK_DLL_MFUN(birlphysicalmodel_noiseBPQ);
CK_DLL_MFUN(birlphysicalmodel_noiseGain);
CK_DLL_MFUN(birlphysicalmodel_shaper);
CK_DLL_MFUN(birlphysicalmodel_shaperMix);
CK_DLL_MFUN(birlphysicalmodel_setTuning);
CK_DLL_MFUN(birlphysicalmodel_setCustomTuning);
//CK_DLL_MFUN(birlphysicalmodel_setNumToneHoles);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(birlphysicalmodel_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT birlphysicalmodel_data_offset = 0;


class BirlPhysicalModel
{
public:
    void debug(std::string msg, double in) {
        if (count % 44100 == 0) {
            printf(msg.c_str(), in);
            count = 0;
        }
    }
    
    void debug(std::string msg, int in) {
        if (count % 44100 == 0) {
            printf(msg.c_str(), in);
            count = 0;
        }
    }
    
    void debug(std::string msg, int in0, double in1) {
        if (count % 44100 == 0) {
            printf(msg.c_str(), in0, in1);
            count = 0;
        }
    }
    
    void debug(std::string msg, double in0, double in1) {
        if (count % 44100 == 0) {
            printf(msg.c_str(), in0, in1);
            count = 0;
        }
    }
    
    void debug(std::string msg, double in0, int in1) {
        if (count % 44100 == 0) {
            printf(msg.c_str(), in0, in1);
            count = 0;
        }
    }
    
    void debug(std::string msg) {
        if (count % 44100 == 0) {
            printf(msg.c_str());
            count = 0;
        }
    }
    
    void debug(std::string msg, double in0, double in1, double in2, double in3) {
        if (count % 44100 == 0) {
            printf(msg.c_str(), in0, in1, in2, in3);
            count = 0;
        }
    }
    
    void checkClip(std::string msg, double input) {
        if (input >= 1 || input <= -1) {
            printf(msg.c_str());
            printf(" is out of bounds!\n");
        }
    }
    
    void tweakRb(int mult) {
        if (originalrb_ * (mult * RB_TWEAK_FACTOR) < 0) {
            return;
        }
        rb_ = originalrb_ + (mult * RB_TWEAK_FACTOR);
        printf("new rb: %f\n", rb_);
        calcTHCoeffs();
    }

    void tweakRth(int mult) {
        int index = toneHoleIndex_;
        if (originalRth_[index] * (mult * RTH_TWEAK_FACTOR) < 0) {
            return;
        }
        rth_[index] = originalRth_[index] + (mult * RTH_TWEAK_FACTOR);
        printf("new rth_[%d]: %f\n", toneHoleIndex_, rth_[index]);
        calcTHCoeffs();
    }

    void tune(double Fc) {
        Fc = (tuning[NUM_NOTES-2]/tuning[NUM_NOTES-1]) * Fc;
        double LS = calcLS(Fc);
        int LC = calcLC(LS);
        double d1 = calcd1(LC, LS);
        printf("LC: %d\n", LC);

        int prevlL = 0;
        int correction = 0;
        // Must handle dummy separately.
        for (int i = 0; i < numTubes_ - 1; i++) {
            tubeLengths_[i] = calclL(d1, i, LS) - prevlL;

            if (i == 0) {
                tubeLengths_[i] -= correction;
            }

            if (tubeLengths_[i] == 0) {
                printf("ERROR: Integer delay line lengths clash!!!!! Use a different tuning or try oversampling.\n");
                return;
            }
            // if (tubes_[i] != NULL) {
            //     printf("WANNA BE FREEEEEEE\n");
            //     freeTube(tubes_[i]);
            // }

            if (i == 0) {
                tubes_[i] = initTube(tubeLengths_[i]);
                prevlL += tubeLengths_[i] + correction;
            } else {
                tubes_[i] = initTube(tubeLengths_[i]);
                prevlL += tubeLengths_[i];
            }
            printf("th %d: lL = %d\n", i, tubeLengths_[i]);
        }

        // Dummy
        tubeLengths_[numTubes_-1] = (int) ((1.0/tuning[numTubes_-1]) * LS) - prevlL;
        if (tubeLengths_[numTubes_-1] == 0) {
            printf("ERROR: Integer delay line lengths clash!!!!! Use a different tuning or try oversampling.\n");
            return;
        }
        tubes_[numTubes_-1] = initTube(tubeLengths_[numTubes_-1]);
        printf("th %d: lL = %d\n", numTubes_-1, tubeLengths_[numTubes_-1]);

        originalrb_ = convertTocm(d1)/200.0;    // main bore radius
        rb_ = originalrb_;
        int lL = tubeLengths_[0];
        for (int i = 0; i < numToneHoles_; i++) {
            originalRth_[i] = convertTocm(calcdH(i, d1, LS, lL))/200.0;
            rth_[i] = originalRth_[i];
            lL += tubeLengths_[i+1];
        }

        printf("rb: %f\n", rb_);
        // printf("tubeLengths:\n");
        // for (int i = 0; i < numTubes_; i++) {
        //     printf("tube %d: %d\n", i, tubeLengths_[i]);
        // }

        printf("toneHole radii:\n");
        lL = 0.0;
        for (int i = 0; i < numToneHoles_; i++) {
            lL += tubeLengths_[i];
            double LSh = (1.0/tuning[i]) * LS;
            printf("th %d rth: %f m, output freq when open: %f\n", i, rth_[i], checkTuning(d1, convertToSamples(rth_[i]*200.0), LSh, lL, calcg(i)));
        }
        calcTHCoeffs();
    }

/* ------ Parameter accessors ----------------------------------------- */
    
    // This method allows setting of the toneHole0 "open-ness" at
    // any point between "Open" (newValue = 1) and "Closed"
    // (newValue = 0).
    void setToneHoleIndex(int index) {
        if (index < 0 || index >= numToneHoles_) {
            printf("index out of range: %d\n", index);
            return;
        }

        toneHoleIndex_ = index;
    }
    
    void setToneHole(double newValue) {
        double new_coeff;
        int index = toneHoleIndex_;
        double thCoeff = thCoeff_[index];
        
        if ( newValue <= 0.0 )
            new_coeff = 0.9995;
        else if ( newValue >= 1.0 )
            new_coeff = thCoeff;
        else
            new_coeff = ( newValue * (thCoeff - 0.9995) ) + 0.9995;
        toneHoles_[index]->setA1( -new_coeff );
        toneHoles_[index]->setB0( new_coeff );
    }

    void setToneHoleRadius(double newRadius) {
        if (newRadius < MIN_TONEHOLE_RADIUS || newRadius > MAX_TONEHOLE_RADIUS) {
            printf("Radius is too big or too small: %f\n", newRadius);
            return;
        }
        int index = toneHoleIndex_;

        rth_[index] = newRadius;
        scatter_[index] = -pow(newRadius,2) / ( pow(newRadius,2) + 2*pow(rb_,2) );

        // Calculate toneHole coefficients.
        double te = newRadius;    // effective length of the open hole
        thCoeff_[index] = (te*2*(SRATE*OVERSAMPLE) - C_m) / (te*2*(SRATE*OVERSAMPLE) + C_m);
    }
    
    void setLengthIndex(int index) {
        if (index < 0 || index >= numTubes_) {
            printf("index out of range: %d\n", index);
            return;
        }

        tubeIndex_ = index;
    }
    
    void setLength(int newlen) {
        if (newlen < 0 || newlen >= MAX_TUBE_LENGTH) {
            printf("length out of range: %d\n", newlen);
            return;
        }
        int index = tubeIndex_;

        tubeLengths_[index] = newlen;
        tubes_[index] = initTube(newlen);
    }
    
    void setBreathPressure(double input) {
        breathPressure_ = input;
    }

    void setLPCutoff(double cut) {
        cut = clip(cut, 30.0, 16000.0);
        printf("lp cutoff: %f\n", cut);
        setCutoffSVF(lp_, cut);
        setCutoffSVF(lp2_, cut);
    }
    
    void setLPQ(double Q) {
        Q = clip(Q, 0.0, 1.0);
        printf("lp Q: %f\n", Q);
        setQSVF(lp_, Q);
        setQSVF(lp2_, Q);
    }

    void setPFCutoff(double cut) {
        cut = clip(cut, 30.0, 16000.0);
        printf("pf cutoff: %f\n", cut);
        setCutoffSVF(pf_, cut);
        setCutoffSVF(pf2_, cut);
    }

    void setPFQ(double Q) {
        Q = clip(Q, 0.0, 1.0);
        printf("pf Q: %f\n", Q);
        setQSVF(pf_, Q);
        setQSVF(pf2_, Q);
    }

    void setNoiseBPCutoff(double cut) {
        cut = clip(cut, 30.0, 16000.0);
        printf("noiseBP cutoff: %f\n", cut);
        setCutoffSVF(noiseBP_, cut);
    }

    void setNoiseGain(double gain) {
        gain = clip(gain, 0.0, 1.0);
        printf("noise gain: %f\n", gain);
        noiseGain_ = gain;
    }

    void setNoiseBPQ(double Q) {
        Q = clip(Q, 0.0, 1.0);
        printf("noiseBP Q: %f\n", Q);
        setQSVF(noiseBP_, Q);
    }

    void setShaperDrive(double d) {
        d = clip(d, 0.0, 1.0);
        printf("m_drive_: %f\n", d);
        m_drive_ = d;
    }

    void setShaperMix(double d) {
        d = clip(d, 0.0, 1.0);
        printf("shaper mix: %f\n", shaperMix_);
        shaperMix_ = d;
    }

    void setTuningWrapper(int t) {
        setTuning((tuningSystem) t);
    }

    void setCustomTuning(double f9, double f8, double f7, double f6, double f5, double f4, double f3, double f2, double f1, double f0, double fn1) {
        double freqs[] = {f9, f8, f7, f6, f5, f4, f3, f2, f1, f0, fn1};
        populateCustomTuning(freqs);
        tune(freqs[NUM_NOTES - 1]);
    }
    
    void calcTHCoeffs() {
        // Calculate initial tone hole three-port scattering coefficients
        for (int i = 0; i < MAX_TONEHOLES; i++) {
            scatter_[i] = -pow(rth_[i],2) / ( pow(rth_[i],2) + 2*pow(rb_,2) );

            // Calculate toneHole coefficients and set for initially open.
            thCoeff_[i] = (rth_[i]*2*(SRATE*OVERSAMPLE) - C_m) / (rth_[i]*2*(SRATE*OVERSAMPLE) + C_m);

            // Initialize tone holes.
            toneHoles_[i] = new stk::PoleZero();
            toneHoles_[i]->setA1(-thCoeff_[i]);
            toneHoles_[i]->setB0(thCoeff_[i]);
            toneHoles_[i]->setB1(-1.0);
        }
    }
    
    // constructor
    BirlPhysicalModel(t_CKFLOAT lowestFrequency)
    {
        numToneHoles_ = MAX_TONEHOLES;
        numTubes_ = MAX_TUBES;
        tubeIndex_ = 0;

        setTuning(EQUAL_TEMPERED);
        tune(440.0);
        
        // reedTable_.setOffset( 0.7 );
        // reedTable_.setSlope( -0.3 );
        
        dcBlocker_ = initDCFilter(0.995);
        dcBlocker2_ = initDCFilter(0.995);
        biquad_ = initBiquad();
        biquadSetCoeffs(biquad_, 0.169301, 0.338601, 0.169301, -0.482013, 0.186622);
        pf_ = initSVF(2000.0, 0.5);
        lp_ = initSVF(5000.0, 0.5);
        pf2_ = initSVF(1000.0, 1.0);
        lp2_ = initSVF(5000.0, 0.5);
        noiseBP_ = initSVF(16000.0, 1.0);
        
        outputGain_ = 1.0;
        noiseGain_ = 0.2;

        breathPressure_ = 0.0;
        count = 0;
        min = 0.0;
        max = 0.0;
        m_drive_ = 0.0;
        shaperMix_ = 0.0;
    }
    
    double interpolateLinear(double a, double b, double alpha) {
        return (alpha * a) + ((1.0-alpha) * b);
    }
    
    // // for Chugins extending UGen
    // SAMPLE tick(SAMPLE in) {
        // return 0.0;
    //     // return inputSVFLP(lp_, noise_.tick());
    // }
    
SAMPLE tick(SAMPLE in)
    {
        double breathInterp[OVERSAMPLE];
        for (int i = 0; i < OVERSAMPLE; i++) {
            breathInterp[i] = interpolateLinear(breathPressure_, prevBreathPressure_, (double) (i+1) / (double) OVERSAMPLE);
        }
        prevBreathPressure_ = breathPressure_;
        
        double pap;
        double pbm;
        double pthm;
        double outsamp = 0.0;
        double scatter;
        double bellReflected;
        
        for (int t = 0; t < OVERSAMPLE; t++) {
            double breath = breathInterp[t];
            double noise = noiseGain_ * (inputSVFBand(noiseBP_, noise_.tick()));
            breath += breath * noise;

            // Calculate the differential pressure = reflected - mouthpiece pressures
            double pressureDiff = accessDelayLine(tubes_[0]->lower) - breath;
            double reedLookup = pressureDiff * reedTable( pressureDiff );
            breath = tanhClip(breath + reedLookup);
            if (breath >= 1 || breath <= -1) {
                printf("breath going out of bounds of -1 to 1: %f\n", breath);
            }

            debug("%d\n", tubeLengths_[0]);
            // Helps reduce high-pitched noise.
            // breath = inputBiquad(biquad_, breath);
            breath = interpolateLinear(shaper(breath, m_drive_), breath, shaperMix_);
            // breath = inputSVFPeak(pf_, breath);
            // breath = inputSVFLP(lp_, breath);
            // breath = inputDCFilter(dcBlocker_, breath);

            for (int i = 0; i < numToneHoles_; i++) {
                // Index in tubes_[] of tube positioned before toneHoles[i].
                int a = i + FRONT_TUBES - 1;
                // Index in tubes_[] of tube positioned after toneHoles[i].
                int b = i + FRONT_TUBES;

                // Three-port junction scattering.
                pap = accessDelayLine(tubes_[a]->upper);
                pbm = accessDelayLine(tubes_[b]->lower);
                pthm = toneHoles_[i]->lastOut();

                scatter = scatter_[i] * (pap + pbm - (2 * pthm));
                pbp_[i] = pap + scatter;
                pam_[i] = pbm + scatter;
                pthp_[i] = pap + scatter + pbm - pthm;
                
                // Sample output at tubes_[SAMPLE_INDEX].
                if (a == SAMPLE_INDEX) {
                    outsamp += pap + pam_[i];
                }

                // Bell reflection at last tube.
                if (i == numToneHoles_ - 1) {
                    double bell = accessDelayLine(tubes_[b]->upper);
                    // double bell2 = shaper(bell1, m_drive_);
                    // double bell4 = inputSVFPeak(pf2_, bell1);
                    // bell = inputSVFLP(lp2_, bell);
                    // // Reflection = Inversion + gain reduction + lowpass filtering.
                    // bell = inputSVFLP(lp2_, bell);
                    // bell = inputDCFilter(dcBlocker2_, bell);
                    bellReflected = bell * -0.995;
                    // bellReflected = filter_.tick(bell * -0.995);
                }
            }

            // Perform all inputs at the end so that we're not altering
            // state prior to calculations.
            for (int i = 0; i < numToneHoles_; i++) {
                // Index in tubes_[] of tube positioned before toneHoles[i].
                int a = i + FRONT_TUBES - 1;
                // Index in tubes_[] of tube positioned after toneHoles[i].
                int b = i + FRONT_TUBES;

                toneHoles_[i]->tick(pthp_[i]);
                inputDelayLine(tubes_[a]->lower, pam_[i]);
                inputDelayLine(tubes_[b]->upper, pbp_[i]);
            }
            inputDelayLine(tubes_[0]->upper, breath);
            inputDelayLine(tubes_[numTubes_-1]->lower, bellReflected);
        }
        
        count++;
        
        // debug("%.5f -0-> ", breath);
        // for (int i = 0; i < numToneHoles_; i++) {
        //     debug("\t%.5f\t-%d->", pbp_[i], i+1);
        // }
        // debug("\n");
        // for (int i = 0; i < numToneHoles_; i++) {
        //     debug("%.5f\t<-%d-\t", pam_[i], i);
        // }
        // debug("%.5f\n", bellReflected);
        // debug("range: %f - %f\n", min, max);
        // debug("\n");
        
        // Clipping, account for oversampling, and gain.
        outsamp /= (double) OVERSAMPLE;
        outsamp = tanhClip(outsamp);
        outsamp *= outputGain_;
        return outsamp;
    }
    
protected:
    int numTubes_;
    int tubeIndex_;
    int numToneHoles_;
    int toneHoleIndex_;
    
    double pam_[MAX_TONEHOLES];
    double pbp_[MAX_TONEHOLES];
    double pthp_[MAX_TONEHOLES];
    double scatter_[MAX_TONEHOLES];
    double thCoeff_[MAX_TONEHOLES];
    
    Tube *tubes_[MAX_TUBES];
    stk::PoleZero *toneHoles_[MAX_TONEHOLES];
    // stk::ReedTable reedTable_;
    stk::OneZero   filter_;
    stk::Noise     noise_;
    DCFilter *dcBlocker_;
    DCFilter *dcBlocker2_;
    Biquad *biquad_;
    Biquad *biquad2_;
    SVF *pf_;
    SVF *lp_;
    SVF *pf2_;
    SVF *lp2_;
    SVF *noiseBP_;
    
    double rb_;
    double originalrb_;
    double rhGain_;
    double outputGain_;
    double noiseGain_;
    double m_drive_;
    double shaperMix_;

    double breathPressure_;
    double prevBreathPressure_;
    
    int count;
    double min, max;
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( BirlPhysicalModel )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "BirlPhysicalModel");
    
    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "BirlPhysicalModel", "UGen");
    
    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, birlphysicalmodel_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, birlphysicalmodel_dtor);
    
    // for UGen's only: add tick function
    QUERY->add_ugen_func(QUERY, birlphysicalmodel_tick, NULL, 1, 1);
    
    // NOTE: if this is to be a UGen with more than 1 channel,
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF
    
    // breathPressure_ getter/setter.
    QUERY->add_mfun(QUERY, birlphysicalmodel_setBreathPressure, "float", "breathPressure");
    QUERY->add_arg(QUERY, "float", "arg0");
    
    // Delay line length getter/setter.
    QUERY->add_mfun(QUERY, birlphysicalmodel_setLength, "int", "length");
    QUERY->add_arg(QUERY, "int", "arg1");
    QUERY->add_arg(QUERY, "int", "arg2");
    
    // toneHole setter
    QUERY->add_mfun(QUERY, birlphysicalmodel_setToneHole, "float", "toneHole");
    QUERY->add_arg(QUERY, "int", "arg4");
    QUERY->add_arg(QUERY, "float", "arg5");

    // toneHole radius setter.
    QUERY->add_mfun(QUERY, birlphysicalmodel_setToneHoleRadius, "float", "toneHoleRadius");
    QUERY->add_arg(QUERY, "int", "arg6");
    QUERY->add_arg(QUERY, "float", "arg7");

    // tune function.
    QUERY->add_mfun(QUERY, birlphysicalmodel_setFundamental, "float", "setFundamental");
    QUERY->add_arg(QUERY, "float", "arg8");

    // rb tweak.
    QUERY->add_mfun(QUERY, birlphysicalmodel_tweakBoreRadius, "int", "tweakBoreRadius");
    QUERY->add_arg(QUERY, "int", "arg9");

    // rth tweak.
    QUERY->add_mfun(QUERY, birlphysicalmodel_tweakToneHoleRadius, "int", "tweakToneHoleRadius");
    QUERY->add_arg(QUERY, "int", "arg10");
    QUERY->add_arg(QUERY, "int", "arg11");

    // Filter cutoff
    QUERY->add_mfun(QUERY, birlphysicalmodel_lpQ, "int", "LPQ");
    QUERY->add_arg(QUERY, "float", "arg21");

    // Filter Q
    QUERY->add_mfun(QUERY, birlphysicalmodel_lpCutoff, "int", "LPcutoff");
    QUERY->add_arg(QUERY, "float", "arg20");

    // Filter cutoff
    QUERY->add_mfun(QUERY, birlphysicalmodel_pfQ, "int", "PFQ");
    QUERY->add_arg(QUERY, "float", "arg22");

    // Filter Q
    QUERY->add_mfun(QUERY, birlphysicalmodel_pfCutoff, "int", "PFcutoff");
    QUERY->add_arg(QUERY, "float", "arg23");

    // NoiseBP cutoff
    QUERY->add_mfun(QUERY, birlphysicalmodel_noiseBPQ, "int", "noiseQ");
    QUERY->add_arg(QUERY, "float", "noiseBPQ");

    // NoiseBP Q
    QUERY->add_mfun(QUERY, birlphysicalmodel_noiseBPCutoff, "int", "noiseCutoff");
    QUERY->add_arg(QUERY, "float", "noiseBPCut");

    // Noise gain
    QUERY->add_mfun(QUERY, birlphysicalmodel_noiseGain, "int", "noiseGain");
    QUERY->add_arg(QUERY, "float", "noiseGain");

    // Shaper
    QUERY->add_mfun(QUERY, birlphysicalmodel_shaper, "int", "shaper");
    QUERY->add_arg(QUERY, "float", "arg15");

    // Shaper mix
    QUERY->add_mfun(QUERY, birlphysicalmodel_shaperMix, "int", "shaperMix");
    QUERY->add_arg(QUERY, "float", "arg16");

    // Tuning
    QUERY->add_mfun(QUERY, birlphysicalmodel_setTuning, "int", "tuning");
    QUERY->add_arg(QUERY, "int", "arg14");
    
    QUERY->add_mfun(QUERY, birlphysicalmodel_setCustomTuning, "int", "setCustomTuning");
    QUERY->add_arg(QUERY, "float", "f9");
    QUERY->add_arg(QUERY, "float", "f8");
    QUERY->add_arg(QUERY, "float", "f7");
    QUERY->add_arg(QUERY, "float", "f6");
    QUERY->add_arg(QUERY, "float", "f5");
    QUERY->add_arg(QUERY, "float", "f4");
    QUERY->add_arg(QUERY, "float", "f3");
    QUERY->add_arg(QUERY, "float", "f2");
    QUERY->add_arg(QUERY, "float", "f1");
    QUERY->add_arg(QUERY, "float", "f0");
    QUERY->add_arg(QUERY, "float", "fn1");

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    birlphysicalmodel_data_offset = QUERY->add_mvar(QUERY, "int", "@bhbp_data", false);
    
    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);
    
    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(birlphysicalmodel_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset) = 0;
    
    // instantiate our internal c++ class representation
    BirlPhysicalModel * bcdata = new BirlPhysicalModel(API->vm->get_srate());
    
    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset) = (t_CKINT) bcdata;
}

// implementation for the destructor
CK_DLL_DTOR(birlphysicalmodel_dtor)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // check it
    if( bcdata )
    {
        // clean up
        delete bcdata;
        OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset) = 0;
        bcdata = NULL;
    }
}

// implementation for tick function
CK_DLL_TICK(birlphysicalmodel_tick)
{
    // get our c++ class pointer
    BirlPhysicalModel * c = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    
    // invoke our tick function; store in the magical out variable
    if(c) *out = c->tick(in);
    
    // yes
    return TRUE;
}

CK_DLL_MFUN(birlphysicalmodel_setBreathPressure)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setBreathPressure(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_setLength)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setLengthIndex(GET_NEXT_INT(ARGS));
    bcdata->setLength(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_setToneHole)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setToneHoleIndex(GET_NEXT_INT(ARGS));
    bcdata->setToneHole(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_setToneHoleRadius)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setToneHoleIndex(GET_NEXT_INT(ARGS));
    bcdata->setToneHoleRadius(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_setFundamental)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->tune(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_tweakBoreRadius)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->tweakRb(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_tweakToneHoleRadius)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setToneHoleIndex(GET_NEXT_INT(ARGS));
    bcdata->tweakRth(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_lpCutoff)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setLPCutoff(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_lpQ)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setLPQ(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_pfCutoff)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setPFCutoff(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_pfQ)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setPFQ(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_noiseBPCutoff)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setNoiseBPCutoff(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_noiseBPQ)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setNoiseBPQ(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_noiseGain)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setNoiseGain(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_shaper)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setShaperDrive(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_shaperMix)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setShaperMix(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_setTuning)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    bcdata->setTuningWrapper(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(birlphysicalmodel_setCustomTuning)
{
    // get our c++ class pointer
    BirlPhysicalModel * bcdata = (BirlPhysicalModel *) OBJ_MEMBER_INT(SELF, birlphysicalmodel_data_offset);
    // set the return value
    double f9 = GET_NEXT_FLOAT(ARGS);
    double f8 = GET_NEXT_FLOAT(ARGS);
    double f7 = GET_NEXT_FLOAT(ARGS);
    double f6 = GET_NEXT_FLOAT(ARGS);
    double f5 = GET_NEXT_FLOAT(ARGS);
    double f4 = GET_NEXT_FLOAT(ARGS);
    double f3 = GET_NEXT_FLOAT(ARGS);
    double f2 = GET_NEXT_FLOAT(ARGS);
    double f1 = GET_NEXT_FLOAT(ARGS);
    double f0 = GET_NEXT_FLOAT(ARGS);
    double fn1 = GET_NEXT_FLOAT(ARGS);
    bcdata->setCustomTuning(f9, f8, f7, f6, f5, f4, f3, f2, f1, f0, fn1);
}
