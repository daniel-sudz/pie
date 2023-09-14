#include <Arduino.h>
#include <Servo.h> 

//      ******************************************************************
//      *                                                                *
//      *                                                                *
//      *     Project 2                                                  *
//      *                                                                *
//      *                                                                *
//      ******************************************************************

// buffer size used for IO
#define BUFFER_SIZE 10000

Servo servo_1;
Servo servo_2;

/*
 * Write to serial using printf formatting string
 * Example: print_format("a:%d, b:%d", a, b)
*/
template <typename ...T>
void print_format(char format[], T... args) {
  char buffer[BUFFER_SIZE]; 
  sprintf(buffer, format, args...);
  Serial.print(buffer);
  while(!Serial.availableForWrite()) {
  };
  Serial.flush();
}

/*
 * Read line from serial to arduino String
 * Example: read_line_format()
*/
template <typename ...T>
String block_read_line_string() {
  // the default serial readStringUntil() function does not block
  // here we block the thread until we can get the whole line
  String res;
  int last;
  while ((last = Serial.read()) != '\n') {
    if (last != -1)
    {
      res += (char)last;
    }
  }
  return res;
}

/*
 * Read line from serial using formatting string
 * Example: read_line_format("%d,%d", &my_int, &my_int)
*/
template <typename ...T>
void block_read_line_format(char format[], T... args) {
  char buffer[BUFFER_SIZE];
  String line = block_read_line_string();
  line.toCharArray(buffer, BUFFER_SIZE);
  sscanf(buffer, format, args...);
}

void setup()
{ 
  // start the serial port, must match the server!!!
  Serial.begin(115200);
}



void loop() 
{
  if(Serial.available()) {
    String command = block_read_line_string();
    if(command.equals("ECHO")) {
      print_format("ECHO\n");
    }
    else if(command.equals("CALIBRATE")) {
      float reading = 2123.12; // reading code goes here
      print_format("%f\n", reading);
    }
    else
    {
      // unrecognized command
    }
  }

}
