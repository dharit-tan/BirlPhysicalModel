//variables to change if necessary:
float myInitialNote; //a temporary note, before any tuning calucaltions happen
36 => myInitialNote; //set initial note
//60 means the low C in the default octave is middle C. 
//48 (60-12) would mean the C below that. 
//49 would mean that note is actually a low C# (and the whole instrument would sound a half-step higher)

.05::ms => dur synthesizerRate;

///GLOBAL VARIABLES ////
11 => int numKeys;
0 => int keysMin;
10 => int keysMax;
250 => int breathMin;
8000 => int breathMax;
float keysFloat[numKeys];

0 => int myoctave;
int scaleType; // normal, otonal or utonal. Read some Harry Partch if you want background on what these mean. Also "the Just Intonation Primer" by David Doty is fantastic. (only $12!)
0 => scaleType; // set it to Normal by default (0 = normal, 1 = otonal, 2 = utonal)
0 => int currentpitch; // what is the note we are playing
-1 => int lastpitch; // 
0 => int tuneNotePos; //these are internal variables for the tuning calculation.
0 => int tuneNoteNeg; //these are internal variables for the tuning calculation.

// these arrays set up the normal, otonal, and utonal scales
[0.0, 1.117313, 2.039101, 3.156414, 3.86314, 4.980453, 5.902237, 7.019547, 8.136864, 8.84359, 9.960899, 10.88269, 12.0] @=> float normalscale[];
[0.0, 1.049553, 2.039101, 2.97128, 3.863136, 4.707809, 5.513176, 7.019547, 8.405273, 9.058647, 9.688255, 10.88269, 12.0] @=> float otonalscale[];
[0.0, 1.11731, 2.311745, 3.15641, 3.594727, 4.980453, 6.486824, 7.292191, 8.136864, 9.024847, 9.960899, 10.950447, 12.0] @=>  float utonalscale[];
[0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0] @=>float equaltemperedscale[];




int breath[2]; 
int negBreath[2];
int XY1_X[2];
int XY1_Y[2];
int XY2_X[2];
int XY2_Y[2];
int mykeys[numKeys];
int breathFull;
int negBreathFull;
int XY1_XFull;
int XY1_YFull;
int XY2_XFull;
int XY2_YFull;
float breathFloat;
float XY1_XFloat;
float XY1_YFloat;
float XY2_XFloat;
float XY2_YFloat;
float myPitch;

// devices to open (try: chuck --probe)
MidiIn min[16];

// number of devices
int devices;
////Here's my signal chain for the synthesis///

// Steps to initialize Birl:
// 1. Set desired tuning system.
// 2. If using custom, populate custom tuning.
// 3. Set a fundamental.

// 0: equal tempered
// 1: just intonation
// 2: meantone
// 3: highland bagpipe
// 4: custom (must call setTuningCustom() with set of 11 freqs first)

BirlPhysicalModel t => dac;

// harmonic minor scale
//(622.25, 587.33, 523.25, 493.88, 415.30, 392.00, 349.23, 311.13, 293.66, 261.63, 246.94) => t.setCustomTuning;
(570.6, 523.25, 479.82, 440.0, 403.48, 369.99, 339.28, 311.13, 285.30, 261.63, 239.9) => t.setCustomTuning;

4 => t.tuning;
239.9 => float Fc;

Fc => t.setFundamental;
0.0 => t.shaper;
0.5 => t.noiseQ;
.5 => t.noiseGain;
200.0 => t.noiseCutoff;

Envelope breathPosEnv => blackhole;
Envelope XY1_XEnv => blackhole;
Envelope XY1_YEnv => blackhole;
Envelope XY2_XEnv => blackhole;
Envelope XY2_YEnv => blackhole;
Envelope keysEnv[11];



//setting the speed of the envelopes for smoothing the parameters
breathPosEnv.duration(3::ms);
XY1_XEnv.duration(5::ms);
XY1_YEnv.duration(5::ms);
XY2_XEnv.duration(5::ms);
XY2_YEnv.duration(5::ms);

for (0 => int i; i < numKeys; i++)
{
    keysEnv[i] => blackhole;
    keysEnv[i].duration(3::ms);
}
///Here come the functions!!!/////



// loop
for( int i; i < min.cap(); i++ )
{
    // no print err
    min[i].printerr( 0 );
    
    // open the device
    if( min[i].open( i ) )
    {
        <<< "device", i, "->", min[i].name(), "->", "open: SUCCESS" >>>;
        spork ~ MidiCollect( min[i], i );
        
        <<<"sporked!">>>;
        devices++;
    }
    else break;
}

// check
if( devices == 0 )
{
    <<< "um, couldn't open a single MIDI device, bailing out..." >>>;
    me.exit();
}



// sets the parameters from the envelope outputs
fun void synthesize()
{
    while(1)
    {

        // 0 is closed, 1 is open. Tone hole 0 is closest to mouthpiece.
        (0, 0.0) => t.toneHole; (1, 1.0-keysEnv[1].value()) => t.toneHole; (2, 1.0-keysEnv[2].value()) => t.toneHole; (3, 1.0-keysEnv[3].value()) => t.toneHole; (4, 1.0-keysEnv[6].value()) => t.toneHole; (5, 1.0-keysEnv[7].value()) => t.toneHole; (6, 1.0-keysEnv[8].value()) => t.toneHole; (7, 1.0 - Math.min(1.0, keysEnv[10].value() + keysEnv[9].value())) => t.toneHole; (8, 1.0-keysEnv[10].value()) => t.toneHole;
        
        breathPosEnv.value() => t.breathPressure;
        
        // Backup plan.
        //0.7 => t.breathPressure;
        .01::ms => now; // this happens frequently
    }
}

// sends the parameters to the envelopes for smoothing
fun void parameterSmooth()
{
    while(1)
    {
        breathFloat => breathPosEnv.target;
        XY1_XFloat => XY1_XEnv.target;
        XY1_YFloat => XY1_YEnv.target;
        XY2_XFloat => XY2_XEnv.target;
        XY2_YFloat => XY2_YEnv.target;
        for (0 => int i; i <  numKeys; i++)
        {
            keysFloat[i] => keysEnv[i].target;
        }
        2::ms => now; // this happens frequently
    }
}


fun void MidiCollect( MidiIn min, int id )
{
    // the message
    MidiMsg msg;
    //<<<"MIDICOLLECT">>>;
    // infinite event loop
    while( true )
    {
        // wait on event
        min => now;
        //<<<"MIDICOLLECT">>>;
        // print message
        while( min.recv( msg ) )
        {    
            if (msg.data1 == 176) {
                // breathPressure
                if (msg.data2 == 30) {
                    (msg.data3 << 7) => breath[0];
                    breath[0] + breath[1] => breathFull;
                    //<<<breathFull>>>;
                }
                if (msg.data2 == 31) {
                    (msg.data3 && 127) => breath[1];
                    breath[0] + breath[1] => breathFull;
                }
                if (msg.data2 == 32) {
                    (msg.data3 << 7) => negBreath[0];
                    negBreath[0] + negBreath[1] => negBreathFull;
                }
                if (msg.data2 == 33) {
                    (msg.data3 && 127) => negBreath[1];
                    negBreath[0] + negBreath[1] => negBreathFull;
                }
                if (msg.data2 == 34) {
                    (msg.data3 << 7) => XY1_Y[0];
                    XY1_Y[0] + XY1_Y[1] => XY1_YFull;
                }
                if (msg.data2 == 35) {
                    (msg.data3 && 127) => XY1_Y[1];
                    XY1_Y[0] + XY1_Y[1] => XY1_YFull;
                }
                if (msg.data2 == 36) {
                    (msg.data3 << 7) => XY1_X[0];
                    XY1_X[0] + XY1_X[1] => XY1_XFull;
                }
                if (msg.data2 == 37) {
                    (msg.data3 && 127) => XY1_X[1];
                    XY1_X[0] + XY1_X[1] => XY1_XFull;
                }
                if (msg.data2 == 38) {
                    (msg.data3 << 7) => XY2_Y[0];
                    XY2_Y[0] + XY2_Y[1] => XY2_YFull;
                }
                if (msg.data2 == 39) {
                    (msg.data3 && 127) => XY2_Y[1];
                    XY2_Y[0] + XY2_Y[1] => XY2_YFull;
                }
                if (msg.data2 == 40) {
                    (msg.data3 << 7) => XY2_X[0];
                    XY2_X[0] + XY2_X[1] => XY2_XFull;
                }
                if (msg.data2 == 41) {
                    (msg.data3 && 127) => XY2_X[1];
                    XY2_X[0] + XY2_X[1] => XY2_XFull;
                }
                for (0 => int i; i < numKeys; i++)
                {
                    if (msg.data2 == 43 + (i * 2)) 
                    {
                        msg.data3 => mykeys[i];
                        //<<<"keys = ", i, mykeys[i]>>>; 
                    }
                }
                
            }
            //<<< "device", id, ":", msg.data1, msg.data2, msg.data3 >>>;
        }
    }
}

fun void scaleValues()
{
    //<<<"here!">>>;
    while(1)
    {
        for (0 => int i; i < numKeys; i++)
        {
            if (mykeys[i] > keysMax)
            {
                keysMax => mykeys[i];
            }
            if (mykeys[i] < keysMin)
            {
                keysMin => mykeys[i];
            }
            (((mykeys[i] $ float) - (keysMin $ float)) / ((keysMax $ float) - (keysMin $ float))) => keysFloat[i];
        }
        
        if (breathFull > breathMax)
        {
            breathMax => breathFull;
        }
        if (breathFull < breathMin)
        {
            breathMin => breathFull;
        }
        (((breathFull $ float) - (breathMin $ float)) / ((breathMax $ float) - (breathMin $ float))) => breathFloat;
        if (breathFloat > 1.0)
        {
            1.0 => breathFloat;
        }
        //<<<"breathFloat =", breathFloat>>>;
        
        ((XY1_XFull $ float) / 255.0) => XY1_XFloat;
        (1.0 - ((XY1_YFull $ float) / 255.0)) => XY1_YFloat;
        ((XY2_XFull $ float) / 255.0) => XY2_XFloat;
        (1.0 - ((XY2_YFull $ float) / 255.0)) => XY2_YFloat;
        10::ms => now;
    }
}
// here's where the magic happens!  This is the program that calls all these functions. "sporked" functions all thread off and execute simultaneously.

//spork ~ MidiCollect( min[1], 0 );
//spork ~ openMidi();
spork ~ synthesize();
spork ~ parameterSmooth();
spork ~ scaleValues();

while(1)
{
    100::second => now;
}
