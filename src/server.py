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
from pathlib import Path

# Intro text
                                                                  
intro_txt = """                                        
  ______ ______ ______ ______ ______ ______ ______ ______ ______  
 |______|______|______|______|______|______|______|______|______| 
 | |  __ \_   _|  ____| |  __ \         (_)         | |   |__ \| |
 | | |__) || | | |__    | |__) | __ ___  _  ___  ___| |_     ) | |
 | |  ___/ | | |  __|   |  ___/ '__/ _ \| |/ _ \/ __| __|   / /| |
 | | |    _| |_| |____  | |   | | | (_) | |  __/ (__| |_   / /_| |
 | |_|   |_____|______| |_|   |_|  \___/| |\___|\___|\__| |____| |
 | |                                   _/ |                    | |
 |_|____ ______ ______ ______ ______ _|__/_ ______ ______ _____|_|
 |______|______|______|______|______|______|______|______|______| 
 """
print(intro_txt)

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


calibration_coefficients = fit_calibration([1, 2, 3, 4], [1, 8, 27, 81])

# determine distance using calibration data
def voltage_to_distance(voltage: float):
  distance = 0
  # evaluate polynomial using naive term summation
  for idx, coef in enumerate(calibration_coefficients):
    distance += coef * (voltage ** (len(calibration_coefficients) - idx))
  return distance

# writes to serial port and then flushes port
def write_with_flush(str: str):
  serialPort.write(bytes(str, "utf8"))
  serialPort.flush()

# calibration data that we recorded
def get_calibration_data():
  cal_distances = [4, 8, 12]
  cal_voltages = []
  calibration_file = Path(__file__).parent.parent / "calibration.txt"
  if(calibration_file.exists()):
    print("located calibration file!, delete calibration.txt to recalibrate")
  else:
    print("-" * 130)
    print(f"calibration file {calibration_file.as_posix()} not found, initiating calibration sequences:\n\n")
    for dist in cal_distances:
        input(f"Position object {dist} inches from sensor, press any button to continue")
        write_with_flush("CALIBRATE\n")
        voltage = serialPort.readline().decode("ascii")
        cal_voltages += voltage
        print(f"Read voltage {voltage}")

get_calibration_data()

print("distance test: ", voltage_to_distance(4))



while True:
  print("loop!")
  write_with_flush("ECHO\n")
  #res = block_readline()
  res = serialPort.readline().decode("ascii")
  print(res)
  time.sleep(5)
