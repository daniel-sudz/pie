
//      ******************************************************************
//      *                                                                *
//      *                                                                *
//      *     Project 2                                                  *
//      *                                                                *
//      *                                                                *
//      ******************************************************************

/*
 * Write to serial using printf formatting string
 * Example: print_format("a:%d, b:%d", a, b)
*/
template <typename ...T>
void print_format(char format[], T... args) {
  char buffer[10000]; 
  sprintf(buffer, format, args...);
  Serial.write(buffer);
}


void setup()
{ 
  // start the serial port, must match the server!!!
  Serial.begin(115200);
}



void loop() 
{  
  while(true) {
    print_format("%d,%d\n", 1, 2);
    delay(400);
  }
}



