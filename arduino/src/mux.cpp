#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

const int output0 = 0;
const int output1 = 1;
const int output2 = 2;
const int output3 = 3;

const int input0 = 4;
const int input1 = 5;
const int input2 = 6;
const int input3 = 7;
const int input4 = 8;
const int input5 = 9;

bool I0[4];
bool I1[4];
bool I2[4];
bool I3[4];
bool I4[4];
bool I5[4];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(output0, OUTPUT);
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);

  pinMode(input0, INPUT);
  pinMode(input1, INPUT);
  pinMode(input2, INPUT);
  pinMode(input3, INPUT);
  pinMode(input4, INPUT);
  pinMode(input5, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  // Output 0
  digitalWrite(output0, HIGH);
  I0[0] = readPin(input0);
  I1[0] = readPin(input1);
  I2[0] = readPin(input2);
  I3[0] = readPin(input3);
  I4[0] = readPin(input4);
  I5[0] = readPin(input5);
  digitalWrite(output0, LOW);
  // Output 1
  digitalWrite(output1, HIGH);
  I0[1] = readPin(input0);
  I1[1] = readPin(input1);
  I2[1] = readPin(input2);
  I3[1] = readPin(input3);
  I4[1] = readPin(input4);
  I5[1] = readPin(input5);
  digitalWrite(output1, LOW);
  // Output 2
  digitalWrite(output2, HIGH);
  I0[2] = readPin(input0);
  I1[2] = readPin(input1);
  I2[2] = readPin(input2);
  I3[2] = readPin(input3);
  I4[2] = readPin(input4);
  I5[2] = readPin(input5);
  digitalWrite(output2, LOW);
  // Output 3
  digitalWrite(output3, HIGH);
  I0[3] = readPin(input0);
  I1[3] = readPin(input1);
  I2[3] = readPin(input2);
  I3[3] = readPin(input3);
  I4[3] = readPin(input4);
  I5[3] = readPin(input5);
  digitalWrite(output3, LOW);

  // Parse boolean arrays
}

// put function definitions here:
bool readPin(int inputPin) {
  if (digitalRead(inputPin) == 'HIGH') {
    return 1;
  }
  else{
    return 0;
  }
}

void printNote(int* inputArray, char* noteArray){
  for(int i = 0; i < 4; i++){
    if(inputArray[i] == 1){
      Serial.println(noteArray[i]);
    }
  }
}