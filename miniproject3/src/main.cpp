/*********************** FIRST PARTY DEPS ***********************/
#include <Arduino.h>
#include <Wire.h>
/*********************** FIRST PARTY DEPS ***********************/

/*********************** THIRD PARTY DEPS ***********************/
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
/*********************** THIRD PARTY DEPS ***********************/

/* GLOBAL STATE */
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor motor1 = *AFMS.getMotor(1);

/* Based on implementation from https://www.wescottdesign.com/articles/pid/pidWithoutAPhd.pdf */
struct PidState {
  // proportional
  float proportional_gain = 0;
  // integral
  float integrator_state = 0;
  float integrator_min = 0;
  float integrator_max = 0;
  float integrator_gain = 0;
  // derivitive
  float derivitive_gain = 0;
  float previous_state = 0;

  // updates the pid state and returns the new motor command
  float update(float error, float current_state) {
    // proportional
    float proportional_term =  error * proportional_gain;

    // integral
    integrator_state += error;
    integrator_state = min(integrator_state, integrator_max);
    integrator_state = max(integrator_state, integrator_min);
    float integral_term = integrator_state * integrator_gain;

    // derivitive
    float derivitive_term = (previous_state - current_state) * derivitive_gain;
    previous_state = current_state;

    return proportional_term + derivitive_term + integral_term;
  }

};

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}