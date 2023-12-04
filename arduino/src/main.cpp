#include <Arduino.h>

// put function declarations here:

const int output0 = 10;
const int output1 = 11;
const int output2 = 2;
const int output3 = 3;

const int input0 = 4;
const int input1 = 5;
const int input2 = 6;
const int input3 = 7;
const int input4 = 8;
const int input5 = 9;

const int pot0 = A0;
const int pot1 = A1;

int pot1_val = 0;
int pot0_val = 0;

const int startPin = 12;
const int endPin = 13;

bool startPressed = 0;
bool endPressed = 0;

const char* notes0[4] = {"130.81", "155.56", "185.00", "220.00"};
const char* notes1[4] = {"138.59", "164.81", "196.00", "233.08"};
const char* notes2[4] = {"146.83", "174.61", "207.65", "246.94"};
const char* notes3[4] = {"261.63", "311.13", "369.99", "440.00"};
const char* notes4[4] = {"277.18", "329.63", "392.00", "466.16"};
const char* notes5[4] = {"293.66", "349.23", "415.30", "493.88"};

// put function definitions here:
bool readPin(int inputPin) {
    if (digitalRead(inputPin) == 'HIGH') {
        return 1;
    } else {
        return 0;
    }
}

void printNote(bool* inputArray, const char** noteArray) {
    for (int i = 0; i < 4; i++) {
        if (inputArray[i] == 1) {
            Serial.print("KEYBOARDNOTE");
            Serial.println(noteArray[i]);
        }
    }
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
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

    pinMode(pot0, INPUT);
    pinMode(pot1, INPUT);
    pinMode(startPin, INPUT);
    pinMode(endPin, INPUT);
}

void loop() {
    // ------ MUX ------
    bool I0[4];
    bool I1[4];
    bool I2[4];
    bool I3[4];
    bool I4[4];
    bool I5[4];
    // Output 0
    digitalWrite(output0, HIGH);
    I0[0] = digitalRead(input0);
    I1[0] = digitalRead(input1);
    I2[0] = digitalRead(input2);
    I3[0] = digitalRead(input3);
    I4[0] = digitalRead(input4);
    I5[0] = digitalRead(input5);
    digitalWrite(output0, LOW);
    // Output 1
    digitalWrite(output1, HIGH);
    I0[1] = digitalRead(input0);
    I1[1] = digitalRead(input1);
    I2[1] = digitalRead(input2);
    I3[1] = digitalRead(input3);
    I4[1] = digitalRead(input4);
    I5[1] = digitalRead(input5);
    digitalWrite(output1, LOW);
    // Output 2
    digitalWrite(output2, HIGH);
    I0[2] = digitalRead(input0);
    I1[2] = digitalRead(input1);
    I2[2] = digitalRead(input2);
    I3[2] = digitalRead(input3);
    I4[2] = digitalRead(input4);
    I5[2] = digitalRead(input5);
    digitalWrite(output2, LOW);
    // Output 3
    digitalWrite(output3, HIGH);
    I0[3] = digitalRead(input0);
    I1[3] = digitalRead(input1);
    I2[3] = digitalRead(input2);
    I3[3] = digitalRead(input3);
    I4[3] = digitalRead(input4);
    I5[3] = digitalRead(input5);
    digitalWrite(output3, LOW);

    // Print notes from boolean arrays
    printNote(I0, notes0);
    printNote(I1, notes1);
    printNote(I2, notes2);
    printNote(I3, notes3);
    printNote(I4, notes4);
    printNote(I5, notes5);

    // ------ etchasketch ------
    startPressed = digitalRead(startPin);
    endPressed = digitalRead(endPin);
    pot0_val = 1023 - analogRead(pot0);
    pot1_val = 1023 - analogRead(pot1);
    Serial.print("POTL");
    Serial.print(pot0_val);
    Serial.print("POTR");
    Serial.println(pot1_val);
    if (startPressed == 1) {
        Serial.println("START");
    }
    if (endPressed == 1) {
        Serial.println("END");
    }
}