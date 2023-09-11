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


# For Windows computers, the name is formatted like: "COM6"
# For Apple computers, the name is formatted like: "/dev/tty.usbmodemfa141"
arduinoComPort = "/dev/tty.usbmodem1201"

# Set the baud rate
baudRate = 115200

# open the serial port
serialPort = serial.Serial(arduinoComPort, baudRate, timeout=1)

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

print("distance test: ", voltage_to_distance(4))

#while True:
#  line = serialPort.readline().decode()
#  print(line)