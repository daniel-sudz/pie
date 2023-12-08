#include <Arduino.h>

// put function declarations here:

// input MUX pins
const int mux_input_pins[] = {2, 3, 4, 5, 6, 7};
const int mux_num_input = sizeof(mux_input_pins) / sizeof(mux_input_pins[0]);

// output MUX pins
const int mux_output_pins[] = {11, 10, 9, 8};
const int mux_num_output = sizeof(mux_output_pins) / sizeof(mux_output_pins[0]);

const int pot0 = A0;
const int pot1 = A1;

const int repress_delay = 10;
const int min_duration = 10;

int pot1_val = 0;
int pot0_val = 0;

const int startPin = 12;
const int endPin = 13;

bool startPressed = 0;
bool endPressed = 0;

unsigned long mux_pressed[mux_num_input][mux_num_output] = {};
unsigned long mux_released[mux_num_input][mux_num_output] = {};

// frequencies for notes on our keyboard
const char* notes[mux_num_input][mux_num_output] = {
    {"130.81", "155.56", "185.00", "220.00"},
    {"138.59", "164.81", "196.00", "233.08"},
    {"146.83", "174.61", "207.65", "246.94"},
    {"261.63", "311.13", "369.99", "440.00"},
    {"277.18", "329.63", "392.00", "466.16"},
    {"293.66", "349.23", "415.30", "493.88"},
};

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    // configure mux output pins
    for (int i = 0; i < mux_num_output; i++) {
        pinMode(mux_output_pins[i], OUTPUT);
    }

    // configure mux input pins
    for (int i = 0; i < mux_num_input; i++) {
        pinMode(mux_input_pins[i], INPUT);
    }

    // configure potentiometer pins
    pinMode(pot0, INPUT);
    pinMode(pot1, INPUT);

    // configure start/stop pins
    pinMode(startPin, INPUT_PULLUP);
    pinMode(endPin, INPUT_PULLUP);
}

void loop() {
  /* ------------------------------------ Etch-a-Sketch ------------------------------ */
    startPressed = digitalRead(startPin);
    endPressed = digitalRead(endPin);
    pot0_val = analogRead(pot0);
    pot1_val = analogRead(pot1);

    Serial.print(pot0_val);
    Serial.print(",");
    Serial.print(pot1_val);

    if (startPressed == 0) {
        Serial.println("START");
    }
    if (endPressed == 0) {
        Serial.println("END");
    }
    /* ------------------------------------ Etch-a-Sketch ------------------------------ */
    /* ------------------------------------  MUX ------------------------------------  */
    // read the mux
    for (int mux_output = 0; mux_output < mux_num_output; mux_output++) {
        digitalWrite(mux_output_pins[mux_output], HIGH);
        for (int mux_input = 0; mux_input < mux_num_input; mux_input++) {
            bool key_down = digitalRead(mux_input_pins[mux_input]);
            if (key_down) {
                if (mux_pressed[mux_input][mux_output] == 0) {
                    if(millis() - mux_released[mux_input][mux_output] > repress_delay) {
                        mux_pressed[mux_input][mux_output] = millis();
                    }
                } else {
                  Serial.print(",");
                  Serial.print(notes[mux_input][mux_output]);
                }
            } else {
                if (mux_pressed[mux_input][mux_output]) {
                    if(millis() - mux_pressed[mux_input][mux_output] > min_duration) {
                        mux_pressed[mux_input][mux_output] = 0;
                    }
                }
            }
        }
        digitalWrite(mux_output_pins[mux_output], LOW);
    }

    Serial.print("\n");

    /* ------------------------------------  MUX ------------------------------------  */
}