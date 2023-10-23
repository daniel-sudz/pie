import os 
from pathlib import Path
import matplotlib.pyplot as plt

# get the path to the plotting data
plot_file_name = os.environ.get("PLOT") or "plot1.data"
plot_file_path = Path(__file__).parent / "data" / plot_file_name
plot_file_contents = plot_file_path.read_text().splitlines()

# group into sensors and commands 
plot_contents_left_sensor_1 = list(float(line.split(",")[0]) for line in plot_file_contents)
plot_contents_left_sensor_2 = list(float(line.split(",")[1]) for line in plot_file_contents)
plot_contents_left_sensor_avg = list((x + y) / 2 for x,y in zip(plot_contents_left_sensor_1, plot_contents_left_sensor_2))

plot_contents_right_sensor_1 = list(float(line.split(",")[2]) for line in plot_file_contents)
plot_contents_right_sensor_2 = list(float(line.split(",")[3]) for line in plot_file_contents)
plot_contents_right_sensor_avg = list((x + y) / 2 for x,y in zip(plot_contents_right_sensor_1, plot_contents_right_sensor_2))

# group into motor commands
plot_contents_left_command = list(float(line.split(",")[4]) for line in plot_file_contents)
plot_contents_right_command = list(float(line.split(",")[5]) for line in plot_file_contents)

# get timestamps 
plot_contents_timestamps= list(float(line.split(",")[6]) for line in plot_file_contents)

# creates and saves a plot of sensors and commands vs time for a specific side 
def save_command_sensor_plot(time: [float], command_left: [float], sensor_left: [float], command_right: [float], sensor_right: [float]):
    fig, ax1 = plt.subplots()
    ax2 = ax1.twinx()

    ax1.plot(time, sensor_left, label = "Left Sensor Reading", color = "blue")
    ax1.plot(time, sensor_right, label = "Right Sensor Reading", color = "orange")
    leg1 = ax1.legend(bbox_to_anchor=(1.625, 1.05))
    
    ax2.plot(time, command_left, label = "Left Motor Command", color = "purple")
    ax2.plot(time, command_right, label = "Right Motor Command", color = "green")
    leg2 = ax2.legend(bbox_to_anchor=(1.2, 0.80))

    ax1.set_xlabel("Time (seconds)")
    
    ax1.xaxis.set_major_locator(plt.MaxNLocator(10))
    ax1.yaxis.set_major_locator(plt.MaxNLocator(10))
    ax2.yaxis.set_major_locator(plt.MaxNLocator(10))

    ax1.set_title('Left/Right Inner Sensors and Motor Commands Over Time')

    ax1.set_ylabel('Inner Sensor analogRead()')
    ax2.set_ylabel('Left/Right Motor Command Sent')

    ax1.set_xlim(left=0, right=15)

# save plots for both sides
save_command_sensor_plot(plot_contents_timestamps, plot_contents_left_command, plot_contents_left_sensor_1,plot_contents_right_command, plot_contents_right_sensor_1)
