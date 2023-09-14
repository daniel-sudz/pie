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
import os
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
  cal_distances = [1] * 10000 #[4, 8, 12]
  cal_voltages = []
  calibration_file = Path(__file__).parent.parent / "calibration.txt"
  if(calibration_file.exists()):
    print("located calibration file!, delete calibration.txt to recalibrate")
  else:
    print("-" * 130)
    print(f"calibration file {calibration_file.as_posix()} not found, initiating calibration sequences:\n\n")
    for dist in cal_distances:
        input(f"Position object {dist} inches from sensor, press any button to continue")
        write_with_flush("READING\n500\n")
        voltage = serialPort.readline().decode("ascii")
        cal_voltages += voltage
        print(f"Read voltage {voltage}")

#get_calibration_data()

print("distance test: ", voltage_to_distance(4))

# records a scan and saves it
def record_scan():
    data = []
    for pan in range(0, 180, 10):
        for tilt in range(0, 180, 10):
            sample_count = 100
            # send tilt angle, tilt angle, and record distance
            write_with_flush(f"PAN\n{pan}\nTILT\n{tilt}\nREADING\n{sample_count}\n")
            voltage = serialPort.readline().decode("ascii")
            data += [(pan, tilt, voltage)]
            if(os.getenv('DEBUG') != None):
                print(f"[SCAN DEBUG]: PAN: {pan}, TILT: {tilt}, VOLTAGE: {voltage}")
    write_with_flush("RESET\n")

    # create filename based on cur time
    cur_date = time.strftime("%Y-%m-%d-%H-%M-%S")
    filename = f"scan-{cur_date}.txt"
    filepath = Path(__file__).parent.parent / "scans" / filename
    
    # save scan to file
    filepath.parent.mkdir(parents=True, exist_ok=True)
    with open(filepath.as_posix(), "w") as file:
        for entry in data:
            file.write(f"{entry[0]},{entry[1]},{entry[3]}\n")




