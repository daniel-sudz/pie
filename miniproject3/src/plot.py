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

plot_contents_right_sensor_1 = list(float(line.split(",")[2]) for line in plot_file_contents)
plot_contents_right_sensor_2 = list(float(line.split(",")[3]) for line in plot_file_contents)

# group into motor commands
plot_contents_left_command = list(float(line.split(",")[4]) for line in plot_file_contents)
plot_contents_right_command = list(float(line.split(",")[5]) for line in plot_file_contents)

# get timestamps 
plot_contents_timestamps= list(float(line.split(",")[4]) for line in plot_file_contents)

# creates and saves a plot of sensors and commands vs time for a specific side 
def save_command_sensor_plot(time: [float], commands: [float], sensor: [float], is_left: bool):
    fig, ax1 = plt.subplots()
    ax2 = ax1.twinx()

    ax1.plot(time, sensor, label = "Sensor Readings", color = "blue")
    ax2.plot(time, commands, label = "Motor Commands", color = "purple")
    ax1.set_xlabel("Time (seconds)")
    
    ax1.xaxis.set_major_locator(plt.MaxNLocator(10))
    ax1.yaxis.set_major_locator(plt.MaxNLocator(10))
    ax2.yaxis.set_major_locator(plt.MaxNLocator(10))

    ax1.set_title(f'{"Left" if is_left else "Right"} Sensor and Motor Commands Over Time')

    ax1.set_ylabel(f'{"Left" if is_left else "Right"} Sensor analogRead()', color='blue')
    ax2.set_ylabel(f'{"Left" if is_left else "Right"} Motor Command Sent', color='purple')

    fig.savefig(f'plots/{"left" if is_left else "right"}.png', dpi=1200)

def save_sensor_vs_sensor_plot(time: [float], sensor1: [float], sensor2: [float]):
    fig = plt.figure()

    plt.plot(time, sensor1, label = "Left", color = "blue")
    plt.plot(time, sensor2, label = "Right", color = "red")

    plt.legend()

    plt.xlabel("Time (seconds)")
    

    fig.savefig(f'plots/sensors.png', dpi=1200)
    plt.show()
    

# save plots for both sides
save_command_sensor_plot(plot_contents_timestamps, plot_contents_left_command, plot_contents_left_sensor_1, True)
save_command_sensor_plot(plot_contents_timestamps, plot_contents_right_command, plot_contents_right_sensor_1, False)
save_sensor_vs_sensor_plot(plot_contents_timestamps, plot_contents_left_sensor_1, plot_contents_right_sensor_1)