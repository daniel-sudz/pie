#      ******************************************************************
#      *                                                                *
#      *                                                                *
#      *    Example Python program that receives data from an Arduino   *
#      *    dependencies: pyserial, numpy, matplotlib                   *
#      *                                                                *
#      *                                                                *
#      ******************************************************************

import serial
import numpy as np
import time
import subprocess
import codecs
import os
import sys
import math
import matplotlib.pyplot as plt
from pathlib import Path


# prints a console info section seperator
def print_break():
   print("-" * 130)

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
print_break()
print(intro_txt)
print_break()

# For Windows computers, the name is formatted like: "COM6"
# For Apple computers, the name is formatted like: "/dev/tty.usbmodemfa141"
arduino_com_port = f'/dev/{subprocess.check_output("ls /dev | grep tty.usbmodem", shell=True, encoding="utf8").strip()}'
print(f"Located serial port at {arduino_com_port}!")

# Set the baud rate
baudRate = 115200

# open the serial port
print("WAITING for serial port to connect!")
serialPort = serial.Serial(arduino_com_port, baudRate, timeout=None)
time.sleep(2) # bug in pyserial library returning before binding: https://stackoverflow.com/a/49429639


# polyfit calibration data of distance sensor using 3rd order polynomial
def fit_calibration(voltage_readings: [float], distance_actual: [float]):
  return np.polyfit(np.log(np.array(voltage_readings)), np.array(distance_actual), 2)

# convert pan/tilt/distance to x,y,z coordinates 
# assumes that pan/tilt are in degrees
# see https://www.mathworks.com/matlabcentral/answers/427558-how-can-i-create-xyz-coordinant-from-pan-tilt-system-angles
def pan_tilt_to_coords(pan: int, tilt: int, distance: float):
    x = distance * math.cos(math.radians(tilt)) * math.cos(math.radians(pan))
    y = distance * math.cos(math.radians(tilt)) * math.sin(math.radians(pan))
    z = distance * math.sin(math.radians(tilt))
    return (x, y, z)

# determine distance using calibration data
def voltage_to_distance(voltage: float, calibration_coefficients: [float]):
  voltage = math.log(voltage)
  distance = 0
  # evaluate polynomial using naive term summation
  for idx, coef in enumerate(calibration_coefficients):
    distance += coef * (voltage ** (len(calibration_coefficients) - idx - 1))
  return distance

# writes to serial port and then flushes port
def write_with_flush(str: str):
  serialPort.write(bytes(str, "utf8"))
  serialPort.flush()

# read a line from serial
def read_line():
   return serialPort.readline().decode("ascii").strip()

# calibration data that we recorded
def record_calibration_data():
  cal_distances = [8, 12, 16, 20, 24, 28, 32, 36]
  cal_voltages = []
  
  # see if calibration file already exists
  calibration_file = Path(__file__).parent.parent / "calibration.txt"
  if(calibration_file.exists()):
    response = input("located calibration file!, (enter y/Y) if you would like to recalibrate: ").strip().lower()
    if(not (response == "y" or response == "Y")):
       return
  
  # record a new calibration file
  print_break()
  print(f"calibration file {calibration_file.as_posix()} not found, initiating calibration sequences:\n\n")
  for dist in cal_distances:
      input(f"Position object {dist} inches from sensor, press any button to continue")
      write_with_flush("READING\n2000\n")
      voltage = read_line()
      cal_voltages += [voltage]
      print(f"Read voltage {voltage}")
      calibration_file.parent.mkdir(parents=True, exist_ok=True)
  with open(calibration_file.as_posix(), "w") as file:
      for idx in range(len(cal_distances)):
          file.write(f"{cal_distances[idx]}, {cal_voltages[idx]}\n")
  print("wrote calibration file!")

# reads the calibration file and returns a function that converts voltage to distance as well as the calibration data itself
# (voltage_to_distance_func, calibration_data)
# if the calibration file does not exist returns None
def read_calibration_data():
   calibration_file = Path(__file__).parent.parent / "calibration.txt"
   if(not calibration_file.exists()):
      return None
   
   calibration_lines = calibration_file.read_text().splitlines()
   calibration_data = list((float(val[0]), float(val[1])) for val in (line.split(',') for line in calibration_lines))
   calibration_coefficients = fit_calibration(list(x[1] for x in calibration_data), list(x[0] for x in calibration_data))

   return ((lambda v: voltage_to_distance(v, calibration_coefficients)), calibration_data)

# records and save a scan
def record_scan():
    # min angle, max angle, and angle step
    pan_range = [60, 180, 1]
    tilt_range = [50, 150, 1]

    print_break()
    print("Starting scan!")

    # go up->down and then down->up to save time while tilting
    tilt_inverse = False

    # record data as we pan/tilt
    data = []

    # how many readings to average on distanse sensor for each measurement
    sample_count = 100

    for pan in range(pan_range[0], pan_range[1], pan_range[2]):
        # swap the carriage return to not waste time
        tilt_inverse = not tilt_inverse
        for tilt in range(tilt_range[0], tilt_range[1], tilt_range[2]):
            tilt = ((tilt_range[1]) - (tilt - tilt_range[0])) if tilt_inverse else tilt
            # send tilt angle, tilt angle, and record distance commands
            write_with_flush(f"PAN\n{pan}\nTILT\n{tilt}\nREADING\n{sample_count}\n")
            voltage = float(read_line().strip())
            if voltage < 600 and voltage > 300:
                data += [(pan, tilt, voltage)]
                sys.stdout.write(f"[SCAN DEBUG RECORD]: PAN: {pan}, TILT: {tilt}, VOLTAGE: {voltage}\r")
            else:
                sys.stdout.write(f"[SCAN DEBUG SKIP]: PAN: {pan}, TILT: {tilt}, VOLTAGE: {voltage}\r")

               
            
    write_with_flush("RESET\n")

    # create filename based on cur time
    cur_date = time.strftime("%Y-%m-%d-%H-%M-%S")
    filename = f"scan-{cur_date}.txt"
    filepath = Path(__file__).parent.parent / "scans" / filename
    
    # save scan to file
    filepath.parent.mkdir(parents=True, exist_ok=True)
    with open(filepath.as_posix(), "w", newline='') as file:
        file.write
        for entry in data:
            file.write(f"{entry[0]},{entry[1]},{entry[2]}\n")

# main control loop
def main():
   response = input(
      """Select a mode below:
            1) Initiate a full scan
            2) Load a previous scan
            3) Calibrate the distance sensor
            4) Send a custom pan command
            5) Send a custom tilt command
            6) Generate a calibration plot visualization
            7) Test the distance sensor
      """
         ).strip()
   if response == "1":
      record_scan()
      print("Scan completed and saved!")
   elif response == "2":
      scan_dir = Path(__file__).parent.parent / "scans"
      found_scans = list(scan_dir.iterdir())
      print_break()
      print(f"Found {len(found_scans)} scans. Select a scan below (press any key to exit): \n")
      for idx, scan in enumerate(found_scans):
         print(f"\t{idx + 1}) {scan.name}")
      response = input().strip()
      if(not response.isdigit() or int(response) < 0 or int(response) > len(found_scans)):
         print("Failed to select scan... returning")
      else:
        voltage_to_distance_data = read_calibration_data()
        if(not voltage_to_distance_data):
            print("Calibration data not found, the sensor needs to be calibrated!")
        else:
          voltage_to_distance_func = voltage_to_distance_data[0]
          voltage_to_distance_data = voltage_to_distance_data[1]

          selected_scan = found_scans[int(response) - 1]
          scan_data_lines = selected_scan.read_text().splitlines()
          scan_data_pan_tilt_voltage = list((
                float(line.split(',')[0]), 
                float(line.split(',')[1]), 
                float(line.split(',')[2]))
             for line in scan_data_lines)
          scan_data_pan_tilt_distance = list((v[0], v[1], voltage_to_distance_func(v[2])) for v in scan_data_pan_tilt_voltage)
          # filter out data points that are too far away from the sensor
          scan_data_pan_tilt_distance = list(v for v in scan_data_pan_tilt_distance if (v[2] < 6))
          scan_data_x_y_z_distance = list(pan_tilt_to_coords(v[0], v[1], v[2]) for v in scan_data_pan_tilt_distance)
          

          print(f"Selected scan {selected_scan}!")

          fig = plt.figure()
          ax = fig.add_subplot(projection='3d')
          ax.scatter(
             list(v[0] for v in scan_data_x_y_z_distance), 
             list(v[1] for v in scan_data_x_y_z_distance),
             list(v[2] for v in scan_data_x_y_z_distance))
          plt.show()

   elif response == "3":
      record_calibration_data()
   elif response == "4":
      angle = int(input("Enter an angle: "))
      if(angle < 0 or angle > 180):
         print("Angle must be between 0 and 180 degrees!")
      else:
         write_with_flush(f"PAN\n{angle}\n")
         print("Command sent!")
   elif response == "5":
      angle = int(input("Enter an angle: "))
      if(angle < 0 or angle > 180):
         print("Angle must be between 0 and 180 degrees!")
      else:
         write_with_flush(f"TILT\n{angle}\n")
         print("Command sent!")
   elif response == "6":
      voltage_to_distance_data = read_calibration_data()
      if(not voltage_to_distance_data):
         print("Calibration data not found, the sensor needs to be calibrated!")
      else:
         voltage_to_distance_func = voltage_to_distance_data[0]
         voltage_to_distance_data = voltage_to_distance_data[1]
         print("Loaded calibration data")
         fig = plt.figure()
         x = list(x for x in range(50,700))
         plt.plot(
            x, 
            list(voltage_to_distance_func(xv) for xv in x), 
            label="Calibration fit function", 
            color="blue")
         plt.scatter(
            list(v[1] for v in voltage_to_distance_data), 
            list(v[0] for v in voltage_to_distance_data), 
            label="Calibration fit raw data", 
            color="blue", 
            facecolor="none")
         plt.xlabel("AnalogRead() from Distance Sensor")
         plt.ylabel("Distance (inches) from Distance Sensor")
         plt.title("Distance Sensor Readings vs Actual Distance Calibration")
         plt.legend()
         plt.show()
   elif response == "7":
      while True:
        samples = 100
        write_with_flush(f"READING\n{samples}\n")
        voltage = read_line()
        print(f"Reading from sensor, averaging {samples} samples: {voltage}")
   else:
      print("Error, response does not match one of the supported modes!")

if __name__ == "__main__":
   read_calibration_data()
   while(True):
      print_break()
      main()
