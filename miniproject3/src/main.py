import serial
import time
import subprocess
import os
import time
import signal
import sys
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

# writes to serial port and then flushes port
def write_with_flush(str: str):
  serialPort.write(bytes(str, "utf8"))
  serialPort.flush()

# debug variables
left_reading = 0
right_reading = 0
left_cmd = 0
right_cmd = 0
start_time = time.time()

# where to save data to 
data_file_name = os.environ.get("PLOT") or "plot1.data"
data_file_path = Path(__file__).parent / "data" / data_file_name

# send the PID constants to the car
write_with_flush("0.005" + "\n")       # speed scale constant
write_with_flush("400" + "\n")         # proportional_gain
write_with_flush("0" + "\n")           # integrator_gain
write_with_flush("0" + "\n")           # integrator_min
write_with_flush("0" + "\n")           # integrator_max
write_with_flush("0" + "\n")           # derivitive_gain

with open(data_file_path.as_posix(), "wt") as save_file: 
   # disable annoying stack trace on ctrl+c 
   signal.signal(signal.SIGINT, lambda _,__: sys.exit(0))

   while(True):
      info = read_line()
      if(info == "READINGS"):
         left_reading = round(float(read_line()))
         right_reading = round(float(read_line()))
      elif(info == "COMMANDS"):
         left_cmd = round(float(read_line()))
         right_cmd = round(float(read_line()))
      cur_time = round(time.time() - start_time, 1)
      save_file.write(f'{left_reading}, {right_reading}, {left_cmd}, {right_cmd}, {cur_time}\n')
      print(f"Left reading {left_reading}, right reading: {right_reading}, Left cmd {left_cmd}, right cmd: {right_cmd}, time: {cur_time}")
