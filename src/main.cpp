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

// ports
#define PAN_PORT 4
#define TILT_PORT 6
#define DIST_SENSOR A0

// offsets for the servos for which angle they are at "zero"
#define PAN_ZERO_OFFSET 0
#define TILT_ZERO_OFFSET 90
#define SERVO_MAX_ANGLE 180

Servo pan_sero;
Servo tilt_servo;

/*
 * Write to serial using printf formatting string
 * Example: print_format("a:%d, b:%d", a, b)
*/
template <typename T>
void print_format(char format[], T args) {
  char buffer[BUFFER_SIZE]; 
  sprintf(buffer, format, args);
  Serial.print(buffer);
  while(!Serial.availableForWrite()) {
  };
  Serial.flush();
}

/*
 * Read line from serial to arduino String
 * Example: read_line_format()
*/
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
template <typename T>
void block_read_line_format(char format[], T args) {
  char buffer[BUFFER_SIZE];
  String line = block_read_line_string();
  line.toCharArray(buffer, BUFFER_SIZE);
  sscanf(buffer, format, args);
}

void setup()
{ 
  // start the serial port, must match the server!!!
  Serial.begin(115200);

  pan_sero.attach(PAN_PORT);
  tilt_servo.attach(TILT_PORT);
  delay(5000);

  // place in a neutral position
  pan_sero.write(PAN_ZERO_OFFSET);
  tilt_servo.write(TILT_ZERO_OFFSET);
  delay(2000);
}

void loop() 
{
  if(Serial.available()) {
    String command = block_read_line_string();
    if(command.equals("ECHO")) {
       Serial.println("ECHO");
    }
    else if(command.equals("READING")) {
      int samples = block_read_line_string().toInt();
      float reading_total = 0;
      for(int i=0; i<samples;i ++) {
        reading_total += analogRead(DIST_SENSOR);
        delay(1);
      }
      float reading_avg = reading_total / samples;
      Serial.println(reading_avg);
    }
    else if(command.equals("TILT")) {
      int angle = block_read_line_string().toInt();
      tilt_servo.write(angle);
      delay(200);
    }
    else if(command.equals("PAN")) {
      int angle = block_read_line_string().toInt();
      pan_sero.write(angle);
      delay(200);
    }
    else if(command.equals("RESET")) {
      pan_sero.write(PAN_ZERO_OFFSET);
      tilt_servo.write(TILT_ZERO_OFFSET);
      delay(2000);
    }
    else
    {
      // unrecognized command
    }
  }

}
