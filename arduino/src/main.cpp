#include <Arduino.h>

const int pot0 = A0;
const int pot1 = A1;

int pot1_val = 0;
int pot0_val = 0;


const int startPin = 12;
const int endPin = 13;

bool startPressed = 0;
bool endPressed = 0;

void setup() {
  Serial.begin(9600);
  pinMode(pot0, INPUT);
  pinMode(pot1, INPUT);
  pinMode(startPin, INPUT);
  pinMode(endPin, INPUT);
}

void loop() {
  startPressed = digitalRead(startPin);
  endPressed  = digitalRead(endPin);
  pot0_val = 1023 - analogRead(pot0);
  pot1_val = 1023 - analogRead(pot1);
  Serial.println("pot0");
  Serial.print(pot0_val);
  Serial.println("pot1");
  Serial.print(pot1_val);
  if (startPressed == 1) {
    Serial.println("START");
  }
  if (endPressed == 1) {
    Serial.println("END");
  }
  
}
