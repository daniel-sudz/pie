import serial
import time
import subprocess
import codecs
import os
import sys
import math
from pathlib import Path


# For Windows computers, the name is formatted like: "COM6"
# For Apple computers, the name is formatted like: "/dev/tty.usbmodemfa141"
arduino_com_port = f'/dev/{subprocess.check_output("ls /dev | grep tty.usbmodem", shell=True, encoding="utf8").strip()}'
print(f"Located serial port at {arduino_com_port}!")

# Set the baud rate
baudRate = 115200

# Open the serial port
print("WAITING for serial port to connect!")
serialPort = serial.Serial(arduino_com_port, baudRate, timeout=5)
time.sleep(2) # bug in pyserial library returning before binding: https://stackoverflow.com/a/49429639

# Writes to serial port and then flushes port
def write_with_flush(str: str):
  serialPort.write(bytes(str, "utf8"))
  serialPort.flush()

# Read a line from serial
def read_line():
   return serialPort.readline().decode("ascii").strip()

while(True):
   info = read_line()
   if(info == "READINGS"):
      left_reading = float(read_line())
      right_reading = float(read_line())
      print(f"Left reading {left_reading}, right reading: {right_reading}")
