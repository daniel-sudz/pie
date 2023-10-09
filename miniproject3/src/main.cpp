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

float left_sensor_1 = 0;
float baseline_left_sensor_1 = 0;
float left_sensor_2 = 0;
float baseline_left_sensor_2 = 0;
float right_sensor_1 = 0;
float baseline_right_sensor_1 = 0;
float right_sensor_2 = 0;
float baseline_right_sensor_2 = 0;
float last_serial_time = millis();
float last_serial_command_time = millis();
float speed_scale_factor = 0;

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
  left_sensor_1 = analogRead(A0);
  right_sensor_1 = analogRead(A2);

  left_sensor_2 = analogRead(A1);
  right_sensor_2 = analogRead(A3);
}

/* Send the sensor readings over serial */
void send_sensor_readings() {
  if(Serial.availableForWrite()) {
    Serial.println("READINGS");
    Serial.println(left_sensor_1);
    Serial.println(left_sensor_2);
    Serial.println(right_sensor_1);
    Serial.println(right_sensor_2);
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
  Serial.setTimeout(1000ULL * 60ULL);

  // start the motor controller 
  AFMS.begin();

  // disable the default full-throttle
  motor_left.setSpeed(0);
  motor_right.setSpeed(0);
  motor_left.run(FORWARD);
  motor_right.run(FORWARD);

  // wait for python to connect
  while(!Serial.available()) {}

  // intitialize PID constants from python
  speed_scale_factor = Serial.parseFloat();
  float proportional_gain = Serial.parseFloat();
  float integrator_gain = Serial.parseFloat();
  float integrator_min = Serial.parseFloat();
  float integrator_max = Serial.parseFloat();
  float derivitive_gain = Serial.parseFloat();

  leftPID.proportional_gain = proportional_gain;
  rightPID.proportional_gain = proportional_gain;

  leftPID.integrator_gain = integrator_gain;
  rightPID.integrator_gain = integrator_gain;

  leftPID.integrator_min = integrator_min;
  rightPID.integrator_min = integrator_min;

  leftPID.integrator_max = integrator_max;
  rightPID.integrator_max = integrator_max;

  leftPID.derivitive_gain = derivitive_gain;
  rightPID.derivitive_gain = derivitive_gain;

  // figure out the reflective baseline
  read_sensor_readings();
  baseline_left_sensor_1 = left_sensor_1;
  baseline_left_sensor_2 = left_sensor_2;
  baseline_right_sensor_1 = right_sensor_1;
  baseline_right_sensor_2 = right_sensor_2;
}

void loop() {
  // reads down to zero when black and around 1000 when white
  read_sensor_readings();

  // send back debug info
  send_sensor_readings();

  // update the PID
  float left_average = (left_sensor_1 + left_sensor_2) / 2;
  float left_average_diff = ((left_sensor_1 - baseline_left_sensor_1) + (left_sensor_2 - baseline_left_sensor_2)) / 2;

  float right_average = (right_sensor_1 + right_sensor_2) / 2;
  float right_average_diff = ((right_sensor_1 - baseline_right_sensor_1) + (right_sensor_2 - baseline_right_sensor_2)) / 2;

  float left_pid_res = leftPID.update(left_average_diff, left_average);
  float right_pid_res = rightPID.update(right_average_diff, right_average);

  // send the motor commands
  // range is [0 - 65,536]
  float max_command = UINT16_MAX;
  float left_command = max((min(max_command - left_pid_res, max_command) * speed_scale_factor), 0.0);  
  float right_command = max((min(max_command - right_pid_res, max_command) * speed_scale_factor * 1.1), 0.0);

  if(left_command > 0) {
    left_command = max(left_command, 300);
  }
  if (right_command > 0) {
    right_command = max(right_command, 300);
  }
    
  send_commands(left_command, right_command);

  // RUN!!! :)

  motor_left.setSpeedFine(left_command);
  motor_right.setSpeedFine(right_command);
  motor_left.run(BACKWARD);
  motor_right.run(BACKWARD);
}
