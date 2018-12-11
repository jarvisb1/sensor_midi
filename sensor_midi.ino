#include "MIDIUSB.h"
#include "PitchToNote.h"

//Don't change these. They're arbitrary values used to set the code to one mode or another.
#define PITCH_CHANGE_MODE (0)
#define VELOCITY_CHANGE_MODE (1)

//Uncomment the line below to enable the serial port for debugging. Comment it out to disable the serial output. NOTE: MIDI will not output in debugging mode because the serial port interferes with it.
//#define SERIAL_DEBUG (1)

#define NUM_SENSORS (2)
const byte sensor_pins[NUM_SENSORS] = {0, 1};
const int sensor_thresholds[NUM_SENSORS] = {100, 100}; //Used in case the sensors read different values
const byte midi_channels[NUM_SENSORS] = {0, 1}; //MIDI channels to output on. I think you can make this the same value if you want the sensors to all drive the same MIDI channel.

#define LOOP_SLEEP_MS (100) // Milliseconds to sleep/delay at the end of each loop iteration.

//Variables for pitch changing mode
#define MIN_PITCH (pitchA0)
#define MAX_PITCH (pitchC8)
#define PITCH_INCREASE_SIZE (1)
#define PITCH_DECREASE_SIZE (1)
#define DEFAULT_PITCH (pitchC3)

//Variables for velocity changing mode
#define MIN_VELOCITY (0)
#define MAX_VELOCITY (127)
#define VELOCITY_INCREASE_SIZE (10)
#define VELOCITY_DECREASE_SIZE (10)
#define DEFAULT_VELOCITY (100)

byte velocities[NUM_SENSORS];
byte pitches[NUM_SENSORS];

//Set this to PITCH_CHANGE_MODE if you want pitch to change up and down. 
//Set this to VELOCITY_CHANGE_MODE if you want velocity to change up and down.
byte mode = VELOCITY_CHANGE_MODE;

void setup() {
  //Initialize the default values
  for (int sensor_num; sensor_num < NUM_SENSORS; sensor_num++) {
    velocities[sensor_num] = DEFAULT_VELOCITY;
    pitches[sensor_num] = DEFAULT_PITCH;
  }

#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  Serial.println("Sensor debug mode (no MIDI will be sent)"); 
#endif
}

int read_value(int sensor_num) {
  int val = 0;
  val = analogRead(sensor_pins[sensor_num]);

#ifdef SERIAL_DEBUG
  Serial.print("Sensor read: "); Serial.println(val);
#endif

  return val;
}

void increase_velocity(int sensor_num) {
  velocities[sensor_num] += VELOCITY_INCREASE_SIZE;
  if (velocities[sensor_num] > MAX_VELOCITY) {
    velocities[sensor_num] = MAX_VELOCITY;
  }
}

void decrease_velocity(int sensor_num) {
  velocities[sensor_num] -= VELOCITY_DECREASE_SIZE;
  if (velocities[sensor_num] < MIN_VELOCITY) {
    velocities[sensor_num] = MIN_VELOCITY;
  }
}

void increase_pitch(int sensor_num) {
  pitches[sensor_num] += PITCH_INCREASE_SIZE;
  if (pitches[sensor_num] > MAX_PITCH) {
    pitches[sensor_num] = MAX_PITCH;
  }
}

void decrease_pitch(int sensor_num) {
  pitches[sensor_num] -= PITCH_DECREASE_SIZE;
  if (pitches[sensor_num] < MIN_PITCH) {
    pitches[sensor_num] = MIN_PITCH;
  }
}


void set_midi(byte channel, byte pitch, byte velocity) {
#ifndef SERIAL_DEBUG //Prevents this following from being executed if in serial debug mode
  midiEventPacket_t noteOn = {0x09, (byte)(0x90 | channel), pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
#else
  Serial.print("MIDI packet velocity: "); Serial.print(velocity); Serial.print(" , pitch: "); Serial.println(pitch);
#endif
}

void loop() {
  for (int sensor_num = 0; sensor_num < NUM_SENSORS; sensor_num++) {
    int new_value = read_value(sensor_num);
    
    if (new_value < sensor_thresholds[sensor_num]) {
      if (mode == VELOCITY_CHANGE_MODE) {
        increase_velocity(sensor_num);
      }
      else {
        increase_pitch(sensor_num);
      }
    } 
    else {
      if (mode == VELOCITY_CHANGE_MODE) {
        decrease_velocity(sensor_num);
      }
      else {
        decrease_pitch(sensor_num);
      }
    }

    set_midi(midi_channels[sensor_num], pitches[sensor_num], velocities[sensor_num]);
  }
  delay(LOOP_SLEEP_MS);
}

