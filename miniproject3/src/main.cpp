/*********************** FIRST PARTY DEPS ***********************/
#include <Arduino.h>
#include <Wire.h>
/*********************** FIRST PARTY DEPS ***********************/

/*********************** THIRD PARTY DEPS ***********************/
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
/*********************** THIRD PARTY DEPS ***********************/

// 4 per float * (10 / second) * 4 variables = 160 bytes / second

/* GLOBAL STATE */
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor motor_left = *AFMS.getMotor(2);
Adafruit_DCMotor motor_right = *AFMS.getMotor(1);

float sensor_baseline = 0;
float left_sensor = 0;
float right_sensor = 0;
float last_serial_time = millis();
float last_serial_command_time = millis();

/* GLOBAL STATE */

/* Based on implementation from https://www.wescottdesign.com/articles/pid/pidWithoutAPhd.pdf */
struct PidState {
  // proportional
  float proportional_gain = 400;
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
  left_sensor = analogRead(A1);
  right_sensor = analogRead(A0);
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

void send_commands(float left_command, float right_command) {
  if(Serial.availableForWrite() && ((millis() - last_serial_command_time) > 500)) {
    Serial.println("COMMANDS");
    Serial.println(left_command);
    Serial.println(right_command);
    last_serial_command_time = millis();
  }
}

void setup() {
  // start the serial port, must match the server!!!
  Serial.begin(115200);

  // start the motor controller 
  AFMS.begin();

  // disable the default full-throttle
  motor_left.setSpeed(0);
  motor_right.setSpeed(0);
  motor_left.run(FORWARD);
  motor_right.run(FORWARD);

  // figure out the reflective baseline
  read_sensor_readings();
  sensor_baseline = (left_sensor + right_sensor) / 2;
}

void loop() {
  // reads down to zero when black and around 1000 when white
  read_sensor_readings();

  // send back debug info
  send_sensor_readings();

  // update the PID
  float left_pid_res = leftPID.update(left_sensor - sensor_baseline, left_sensor);
  float right_pid_res = rightPID.update(right_sensor - sensor_baseline, right_sensor);

  // send the motor commands
  // range is [0 - 65,536]
  float speed_scale_factor = 0.005;
  float max_command = UINT16_MAX;
  float left_command = max((min(max_command - left_pid_res, max_command) * speed_scale_factor * 1.2), 0.0);  
  float right_command = max((min(max_command - right_pid_res, max_command) * speed_scale_factor), 0.0);

  send_commands(left_command, right_command);

  // RUN!!! :)
  
  motor_left.setSpeedFine(left_command);
  motor_right.setSpeedFine(right_command);
  motor_left.run(FORWARD);
  motor_right.run(FORWARD);
  
}
