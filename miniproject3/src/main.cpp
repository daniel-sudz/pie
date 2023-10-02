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
Adafruit_DCMotor motor_left = *AFMS.getMotor(1);
Adafruit_DCMotor motor_right = *AFMS.getMotor(2);

float left_sensor = 0;
float right_sensor = 0;
float last_serial_time = millis();

/* GLOBAL STATE */

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

PidState leftPID;
PidState rightPID;

/* Reads the onboard sensors and saves them to global state */
void read_sensor_readings() {
  left_sensor = analogRead(A0);
  right_sensor = analogRead(A1);
}

/* Send the sensor readings over serial */
void send_sensor_readings() {
  if(Serial.availableForWrite() && ((millis() - last_serial_time) > 500)) {
    Serial.println("READINGS");
    Serial.println(left_sensor);
    Serial.println(right_sensor);
    last_serial_time = millis();
  }
}

void setup() {
  // start the serial port, must match the server!!!
  Serial.begin(115200);

  // set the PID constants
  leftPID.derivitive_gain = 0;
  rightPID.derivitive_gain = 0;

  leftPID.integrator_gain = 0;
  rightPID.integrator_gain = 0;

  leftPID.integrator_min = 200;
  rightPID.integrator_min = 200;

  leftPID.integrator_max = 200;
  rightPID.integrator_max = 200;

  leftPID.integrator_state = 0;
  rightPID.integrator_state = 0;

  leftPID.proportional_gain = 10;
  rightPID.proportional_gain = 10;
}

void loop() {
  read_sensor_readings();
  send_sensor_readings();

  // update the PID
  float left_pid_res = leftPID.update(left_sensor, left_sensor);
  float right_pid_res rightPID.update(right_sensor, right_sensor);

  // send the motor commands
  // range is [0 - 65,536]
  motor_left.setSpeedFine(min(left_pid_res, UINT16_MAX));
  motor_right.setSpeedFine(min(right_pid_res, UINT16_MAX));
}
