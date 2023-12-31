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

const int erasePin = 12;
const int susPin = 13;

bool erase_up = 0;
bool sus_up = 0;

unsigned long mux_pressed[mux_num_input][mux_num_output] = {};
unsigned long mux_released[mux_num_input][mux_num_output] = {};

unsigned long erase_pressed = 0;
unsigned long erase_released = 0;
unsigned long sus_pressed = 0;
unsigned long sus_released = 0;

bool sus_nc;

/* Marks a note as being pressed using a bitfield approach */
void set_note_field(uint32_t& note, uint32_t mux_input, uint32_t mux_output) {
    uint32_t field = (uint32_t(1) << uint32_t(((mux_input * mux_num_output) + mux_output)));
    note |= field;
}

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
    pinMode(erasePin, INPUT_PULLUP);
    pinMode(susPin, INPUT_PULLUP);

    // figure out if pedal is normally closed (nc)
    sus_nc = !digitalRead(susPin);
}

void loop() {
    /* ------------------------------------ Etch-a-Sketch ------------------------------ */
    erase_up = digitalRead(erasePin);
    sus_up = digitalRead(susPin);
    pot0_val = analogRead(pot0);
    pot1_val = analogRead(pot1);

    if (erase_up == 0) {
        if (erase_pressed == 0) {
            if (millis() - erase_released > repress_delay) {
                Serial.println("ERASE");
                erase_pressed = millis();
            }
        }
    } else {
        if (millis() - erase_pressed > min_duration) {
            erase_pressed = 0;
            erase_released = millis();
        }
    }

    if (sus_up == sus_nc) {
        if (sus_pressed == 0) {
            if (millis() - sus_released > repress_delay) {
                sus_pressed = millis();
            }
        }
    } else {
        if (millis() - sus_pressed > min_duration) {
            sus_pressed = 0;
            sus_released = millis();
        }
    }
    Serial.print("POTL");
    Serial.print(pot0_val);
    Serial.print("POTR");
    Serial.println(pot1_val);

    /* ------------------------------------ Etch-a-Sketch ------------------------------ */
    /* ------------------------------------ MUX ------------------------------------ */
    // read the mux
    uint32_t notes_pressed = 0;
    for (int mux_output = 0; mux_output < mux_num_output; mux_output++) {
        digitalWrite(mux_output_pins[mux_output], HIGH);
        for (int mux_input = 0; mux_input < mux_num_input; mux_input++) {
            bool key_down = digitalRead(mux_input_pins[mux_input]);
            if (key_down) {
                if (mux_pressed[mux_input][mux_output] == 0) {
                    if (millis() - mux_released[mux_input][mux_output] > repress_delay) {
                        mux_pressed[mux_input][mux_output] = millis();
                    }
                } else {
                    set_note_field(notes_pressed, (uint32_t)mux_input, (uint32_t)mux_output);
                }
            } else if (sus_pressed == 0) {
                if (mux_pressed[mux_input][mux_output]) {
                    if (millis() - mux_pressed[mux_input][mux_output] > min_duration) {
                        mux_pressed[mux_input][mux_output] = 0;
                        mux_released[mux_input][mux_output] = millis();
                    }
                }
            } else {
                if (mux_pressed[mux_input][mux_output]) {
                    set_note_field(notes_pressed, (uint32_t)mux_input, (uint32_t)mux_output);
                }
            }
        }
        digitalWrite(mux_output_pins[mux_output], LOW);
    }

    /* print the notes being pressed */
    Serial.print("KEYBOARDNOTES");
    Serial.println(notes_pressed);
    /* ------------------------------------  MUX ------------------------------------  */
}
