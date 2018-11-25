#include "MIDIUSB.h"
#include "PitchToNote.h"

//Don't change these. They're arbitrary values used to set the code to one mode or another.
#define PITCH_CHANGE_MODE (0)
#define VELOCITY_CHANGE_MODE (1)

//Uncomment the line below to enable the serial port for debugging. Comment it out to disable the serial output. NOTE: MIDI will not output in debugging mode because the serial port interferes with it.
//#define SERIAL_DEBUG (1)

#define SENSOR_IN_PIN (0)
#define SENSOR_VALUE_THRESHOLD (100) //This sensor is actually more of a binary "on/off" reading of 1023 or 0, but this threshold still allows the code below to work.
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

int velocity = DEFAULT_VELOCITY;
byte pitch = DEFAULT_PITCH;

//Set this to PITCH_CHANGE_MODE if you want pitch to change up and down. 
//Set this to VELOCITY_CHANGE_MODE if you want velocity to change up and down.
byte mode = VELOCITY_CHANGE_MODE;

byte channel = 0; //MIDI channel to output on. I'm not sure what happens if you change this.

void setup() {
#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }
  Serial.println("Sensor debug mode (no MIDI will be sent)"); 
#endif
}

int read_value() {
  int val = 0;
  val = analogRead(SENSOR_IN_PIN);

#ifdef SERIAL_DEBUG
  Serial.print("Sensor read: "); Serial.println(val);
#endif

  return val;
}

void increase_velocity() {
  velocity += VELOCITY_INCREASE_SIZE;
  if (velocity > MAX_VELOCITY) {
    velocity = MAX_VELOCITY;
  }
}

void decrease_velocity() {
  velocity -= VELOCITY_DECREASE_SIZE;
  if (velocity < MIN_VELOCITY) {
    velocity = MIN_VELOCITY;
  }
}

void increase_pitch() {
  pitch += PITCH_INCREASE_SIZE;
  if (pitch > MAX_PITCH) {
    pitch = MAX_PITCH;
  }
}

void decrease_pitch() {
  pitch -= PITCH_DECREASE_SIZE;
  if (pitch < MIN_PITCH) {
    pitch = MIN_PITCH;
  }
}


void set_midi() {
#ifndef SERIAL_DEBUG //Prevents this following from being executed if in serial debug mode
  midiEventPacket_t noteOn = {0x09, (byte)(0x90 | channel), pitch, (byte)velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
#else
  Serial.print("MIDI packet velocity: "); Serial.print((byte)velocity); Serial.print(" , pitch: "); Serial.println((byte)pitch);
#endif
}

void loop() {
  int new_value = read_value();
  
  if (new_value < SENSOR_VALUE_THRESHOLD) {
    if (mode == VELOCITY_CHANGE_MODE) {
      increase_velocity();
    }
    else {
      increase_pitch();
    }
  } 
  else {
    if (mode == VELOCITY_CHANGE_MODE) {
      decrease_velocity();
    }
    else {
      decrease_pitch();
    }
  }

  set_midi();
  delay(LOOP_SLEEP_MS);
}

