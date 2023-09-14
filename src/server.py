#      ******************************************************************
#      *                                                                *
#      *                                                                *
#      *    Example Python program that receives data from an Arduino   *
#      *    dependencies: pyserial, numpy                               *
#      *                                                                *
#      *                                                                *
#      ******************************************************************

import serial
import numpy as np
import time
import subprocess
import codecs



# For Windows computers, the name is formatted like: "COM6"
# For Apple computers, the name is formatted like: "/dev/tty.usbmodemfa141"
arduino_com_port = f'/dev/{subprocess.check_output("ls /dev | grep tty.usbmodem", shell=True, encoding="utf8").strip()}'
print(f"Located serial port at {arduino_com_port}!")

# Set the baud rate
baudRate = 115200

# open the serial port
print("WAITING for serial port to connect!")
serialPort = serial.Serial(arduino_com_port, baudRate, timeout=None)
time.sleep(2) # bug in pyserial library returning before binding: https://stackoverflow.com/a/49429639\


# polyfit calibration data of distance sensor using 3rd order polynomial
def fit_calibration(voltage_readings: [float], distance_actual: [float], ):
  return np.polyfit(np.array(voltage_readings), np.array(distance_actual), 3)

# calibration data that we recorded
calibration_coefficients = fit_calibration([1, 2, 3, 4], [1, 8, 27, 81])

# determine distance using calibration data
def voltage_to_distance(voltage: float):
  distance = 0
  # evaluate polynomial using naive term summation
  for idx, coef in enumerate(calibration_coefficients):
    distance += coef * (voltage ** (len(calibration_coefficients) - idx))
  return distance

def write_with_flush(str: str):
  serialPort.write(bytes(str, "utf8"))
  serialPort.flush()

print("distance test: ", voltage_to_distance(4))


# sometimes the built in pyserial readline does not block or reads incomplete
# values even with timeout=None set on the port
def block_readline() -> str:
  ret = b''
  last = b'0'
  while(last != b'\n'):
    print(ret)
    last = serialPort.read(1)
    ret += last
  print(ret)
  return bytearray(ret).decode("utf8")


while True:
  print("loop!")
  write_with_flush("ECHO\n")
  #res = block_readline()
  res = serialPort.readline().decode("utf8")
  print(res)
  time.sleep(5)
