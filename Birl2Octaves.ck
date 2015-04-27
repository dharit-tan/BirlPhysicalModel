// opens MIDI input devices one by one, starting from 0,
// until it reaches one it can't open.  then waits for
// midi events on all open devices and prints out the
// device, and contents of the MIDI message

// devices to open (try: chuck --probe)
MidiIn min[16];

// number of devices
int devices;

// loop
for( int i; i < min.cap(); i++ )
{
    // no print err
    min[i].printerr( 0 );
    
    // open the device
    if( min[i].open( i ) )
    {
        <<< "device", i, "->", min[i].name(), "->", "open: SUCCESS" >>>;
        spork ~ go( min[i], i );
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

BlowHoleBiCVariableTH t => Gain g1 => dac;
BlowHoleBiCVariableTH t2 => Gain g2 => dac;
//(0, 18) => t.length; (1, 1) => t.length; (2, 2) => t.length; (3, 1) => t.length; (4, 3) => t.length; (5, 3) => t.length; (6, 3) => t.length; (7, 2) => t.length; (8, 4) => t.length; (9, 4) => t.length;
//(0, 0.025115) => t.toneHoleRadius; (1, 0.025309) => t.toneHoleRadius; (2, 0.021287) => t.toneHoleRadius; (3, 0.016874) => t.toneHoleRadius; (4, 0.029758) => t.toneHoleRadius; (5, 0.021181) => t.toneHoleRadius; (6, 0.020706) => t.toneHoleRadius; (7, 0.018864) => t.toneHoleRadius; (8, 0.027862) => t.toneHoleRadius;

// Steps to initialize Birl:
// 1. Set desired tuning system.
// 2. If using custom, populate custom tuning.
// 3. Set a fundamental.

// harmonic minor scale
//(622.25, 587.33, 523.25, 493.88, 415.30, 392.00, 349.23, 311.13, 293.66, 261.63, 246.94) => t.setCustomTuning;

// 0: equal tempered
// 1: just intonation
// 2: meantone
// 3: highland bagpipe
// 4: custom (must call setTuningCustom() with set of 11 freqs first)
3 => t.tuning;
//446 => float Fc;
246.94 => float Fc;
Fc => t.setFundamental;
Fc*2 => t2.setFundamental;

1.0 => float interp;

// infinite time loop
while( true ) 1::second => now;


// handler for one midi event
fun void go( MidiIn min, int id )
{
    // the message
    MidiMsg msg;
    
    // infinite event loop
    while( true )
    {
        // wait on event
        min => now;
        
        // print message
        while( min.recv( msg ) )
        {
            // Interpolation between two octaves.
            interp => g1.gain;
            (1.0 - interp) => g2.gain;
            //<<< "g1 gain: " + g1.gain()>>>;
            //<<< "g2 gain: " + g2.gain()>>>;
            
            if (msg.data1 == 176) {
                // breathPressure
                if (msg.data2 == 1) {
                    msg.data3/64.0 => t.breathPressure;
                    msg.data3/64.0 => t2.breathPressure;
                }
                
                // PF Q
                if (msg.data2 == 16) {
                    (msg.data3 - 64.0)/64.0 => float a;
                    Math.pow(2.0, a*7) => t.PFQ;
                    Math.pow(2.0, a*7) => t2.PFQ;
                }
                // PF cut
                if (msg.data2 == 17) {
                    (msg.data3 - 64.0)/64.0 => float a;
                    Math.pow(2.0, a*15) => t.PFcutoff;
                    Math.pow(2.0, a*15) => t2.PFcutoff;
                }
                // LP Q
                if (msg.data2 == 18) {
                    (msg.data3 - 64.0)/64.0 => float a;
                    Math.pow(2.0, a*5) => t.LPQ;
                    Math.pow(2.0, a*5) => t2.LPQ;
                }
                // LP cut
                if (msg.data2 == 19) {
                    Std.mtof(msg.data3) => float a;
                    a => t.LPcutoff;
                    a => t2.LPcutoff;
                }
                if (msg.data2 == 107) {
                    Std.mtof(msg.data3) => t.noiseCutoff;
                    Std.mtof(msg.data3) => t2.noiseCutoff;
                }
                if (msg.data2 == 47) {
                    (msg.data3 - 64.0)/64.0 => float a;
                    Math.pow(2.0, a*7) => t.noiseQ;
                    Math.pow(2.0, a*7) => t2.noiseQ;
                }
                if (msg.data2 == 61) {
                    msg.data3/127.0 => t.noiseGain;
                    msg.data3/127.0 => t2.noiseGain;
                }
                if (msg.data2 == 3) {
                    <<< "interp: ", msg.data3/127.0>>>;
                    msg.data3/127.0 => interp;
                }
                
                // shaper drive
                if (msg.data2 == 7) {
                    msg.data3/127.0 => t.shaper;
                    msg.data3/127.0 => t2.shaper;
                }
                
                /*
                // Bore tweak
                if (msg.data2 == 9) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<<b>>>;
                    b => t.tweakBoreRadius;
                }
                // TH tweaks
                if (msg.data2 == 16) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (0, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 17) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (1, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 18) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (2, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 19) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (3, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 3) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (4, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 61) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (5, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 47) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (6, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 107) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (7, b) => t.tweakToneHoleRadius;
                }
                if (msg.data2 == 7) {
                    (msg.data3 - 64.0) $ int => int b;
                    <<< b >>>;
                    (8, b) => t.tweakToneHoleRadius;
                }*/
                
            }
            else if (msg.data1 == 144) {
                if (msg.data2 == 72) {
                    if (msg.data3 == 0.0) {
                        (0, 1.0) => t.toneHole;
                        (0, 1.0) => t2.toneHole;
                    }
                    else {
                        (0, 0.0) => t.toneHole;
                        (0, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 74) {
                    if (msg.data3 == 0.0) {
                        (1, 1.0) => t.toneHole;
                        (1, 1.0) => t2.toneHole;
                    }
                    else {
                        (1, 0.0) => t.toneHole;
                        (1, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 76) {
                    if (msg.data3 == 0.0) {
                        (2, 1.0) => t.toneHole;
                        (2, 1.0) => t2.toneHole;
                    }
                    else {
                        (2, 0.0) => t.toneHole;
                        (2, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 77) {
                    if (msg.data3 == 0.0) {
                        (3, 1.0) => t.toneHole;
                        (3, 1.0) => t2.toneHole;
                    }
                    else {
                        (3, 0.0) => t.toneHole;
                        (3, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 79) {
                    if (msg.data3 == 0.0) {
                        (4, 1.0) => t.toneHole;
                        (4, 1.0) => t2.toneHole;
                    }
                    else {
                        (4, 0.0) => t.toneHole;
                        (4, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 81) {
                    if (msg.data3 == 0.0) {
                        (5, 1.0) => t.toneHole;
                        (5, 1.0) => t2.toneHole;
                    }
                    else {
                        (5, 0.0) => t.toneHole;
                        (5, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 83) {
                    if (msg.data3 == 0.0) {
                        (6, 1.0) => t.toneHole;
                        (6, 1.0) => t2.toneHole;
                    }
                    else {
                        (6, 0.0) => t.toneHole;
                        (6, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 84) {
                    if (msg.data3 == 0.0) {
                        (7, 1.0) => t.toneHole;
                        (7, 1.0) => t2.toneHole;
                    }
                    else {
                        (7, 0.0) => t.toneHole;
                        (7, 0.0) => t2.toneHole;
                    }
                }
                if (msg.data2 == 86) {
                    if (msg.data3 == 0.0) {
                        (8, 1.0) => t.toneHole;
                        (8, 1.0) => t2.toneHole;
                    }
                    else {
                        (8, 0.0) => t.toneHole;
                        (8, 0.0) => t2.toneHole;
                    }
                }
            }
            // print out midi message with id
            //<<< "device", id, ":", msg.data1, msg.data2, msg.data3 >>>;
        }
    }
}
