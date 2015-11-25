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

// Steps to initialize Birl:
// 1. Set desired tuning system.
// 2. If using custom, populate custom tuning.
// 3. Set a fundamental.

// 0: equal tempered
// 1: just intonation
// 2: meantone
// 3: highland bagpipe
// 4: custom (must call setTuningCustom() with set of 11 freqs first)

BirlPhysicalModel t => Gain g1 => dac;

// harmonic minor scale
(622.25, 587.33, 523.25, 493.88, 415.30, 392.00, 349.23, 311.13, 293.66, 261.63, 246.94) => t.setCustomTuning;

0 => t.tuning;
440 => float Fc;
Fc => t.setFundamental;

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
        
        0.5 => t.breathPressure;
        
        // print message
        while( min.recv( msg ) )
        {    
            if (msg.data1 == 176) {

                if (msg.data2 == 107) {
                    Std.mtof(msg.data3) => t.noiseCutoff;
                }
                if (msg.data2 == 47) {
                    (msg.data3 - 64.0)/64.0 => float a;
                    Math.pow(2.0, a*7) => t.noiseQ;
                }
                if (msg.data2 == 61) {
                    msg.data3/127.0 => t.noiseGain;
                }
                
                // shaper drive
                if (msg.data2 == 16) {
                    msg.data3/127.0 => t.shaper;
                }
                // shaper mix
                if (msg.data2 == 17) {
                    msg.data3/127.0 => t.shaperMix;
                }
            }
            
            else if (msg.data1 == 128) {
                // breathPressure
                if (msg.data2 == 49) {
                    0.2 => t.breathPressure;
                }
                if (msg.data2 == 48) {
                        (0, 1.0) => t.toneHole;
                }
                if (msg.data2 == 50) {
                        (1, 1.0) => t.toneHole;
                }
                if (msg.data2 == 52) {
                        (2, 1.0) => t.toneHole;
                }
                if (msg.data2 == 53) {
                        (3, 1.0) => t.toneHole;
                }
                if (msg.data2 == 55) {
                        (4, 1.0) => t.toneHole;
                }
                if (msg.data2 == 57) {
                        (5, 1.0) => t.toneHole;
                }
                if (msg.data2 == 59) {
                        (6, 1.0) => t.toneHole;
                }
                if (msg.data2 == 60) {
                        (7, 1.0) => t.toneHole;
                }
                if (msg.data2 == 62) {
                        (8, 1.0) => t.toneHole;
                }
            }
            
            else if (msg.data1 == 144) {
                // breathPressure
                if (msg.data2 == 49) {
                    1.0 => t.breathPressure;
                }
                if (msg.data2 == 48) {
                        (0, 0.0) => t.toneHole;
                }
                if (msg.data2 == 50) {
                        (1, 0.0) => t.toneHole;
                }
                if (msg.data2 == 52) {
                        (2, 0.0) => t.toneHole;
                }
                if (msg.data2 == 53) {
                        (3, 0.0) => t.toneHole;
                }
                if (msg.data2 == 55) {
                        (4, 0.0) => t.toneHole;
                }
                if (msg.data2 == 57) {
                        (5, 0.0) => t.toneHole;
                }
                if (msg.data2 == 59) {
                        (6, 0.0) => t.toneHole;
                }
                if (msg.data2 == 60) {
                        (7, 0.0) => t.toneHole;
                }
                if (msg.data2 == 62) {
                        (8, 0.0) => t.toneHole;
                }
            }

            // print out midi message with id
            <<< "device", id, ":", msg.data1, msg.data2, msg.data3 >>>;
        }
    }
}
