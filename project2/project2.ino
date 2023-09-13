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

// commands that we know
String SEND_SERVO("SEND_SERVO");

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
  Serial.write(buffer);
}

/*
 * Read line from serial using formatting string
 * Example: read_line_format("%d,%d", &my_int, &my_int)
*/
template <typename ...T>
void read_line_format(char format[], T... args) {
  char buffer[BUFFER_SIZE]; 
  String line = Serial.readStringUntil('\n');
  line.toCharArray(buffer, BUFFER_SIZE);
  sscanf(buffer, format, args...);
}

/*
 * Read line from serial to arduino String
 * Example: read_line_format()
*/
template <typename ...T>
String read_line_string() {
  return Serial.readStringUntil('\n');
}

void setup()
{ 
  // start the serial port, must match the server!!!
  Serial.begin(115200);

  // servo init
  servo_1.attach(6);
  delay(5 * 1000);
}



void loop() 
{  
    // read the command
    String command = read_line_string();
    if(command.equals(SEND_SERVO)) {
      int angle;
      read_line_format("%d", &angle);
      servo_1.write(angle);
      delay(2000);
    }
}



