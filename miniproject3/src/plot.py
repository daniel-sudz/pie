import os 
from pathlib import Path
import matplotlib.pyplot as plt


plot_file_name = os.environ.get("PLOT") or "plot1.data"
plot_file_path = Path(__file__).parent / "data" / plot_file_name
plot_file_contents = plot_file_path.read_text().splitlines()

# group into sensors and commands 
plot_contents_left_sensor = list(line.split(",")[0] for line in plot_file_contents)
plot_contents_right_sensor = list(line.split(",")[1] for line in plot_file_contents)

# group into motor commands
plot_contents_left_command = list(line.split(",")[2] for line in plot_file_contents)
plot_contents_right_command = list(line.split(",")[3] for line in plot_file_contents)

# get timestamps 
plot_contents_timestamps= list(line.split(",")[4] for line in plot_file_contents)

# creates and saves a plot of sensors and commands vs time for a specific side 
def save_command_sensor_plot(time: [float], commands: [float], sensor: [float], is_left: bool):
    fig, ax1 = plt.subplots()
    ax2 = ax1.twinx()

    ax2.plot(time, commands, label = "Motor Commands", color = "purple")
    ax1.plot(time, sensor, label = "Sensor Readings", color = "blue")
    ax1.set_xlabel("Time (milliseconds)")

    ax1.set_title(f'{"Left" if is_left else "Right"} Sensor and Motor Commands Over Time')

    ax1.set_ylabel(f'{"Left" if is_left else "Right"} Sensor analogRead()', color='blue')
    ax2.set_ylabel(f'{"Left" if is_left else "Right"} Motor Command Sent', color='purple')

    fig.savefig(f'plots/{"left" if is_left else "right"}.png', dpi=1200)


save_command_sensor_plot(plot_contents_timestamps, plot_contents_left_command, plot_contents_left_sensor, True)
save_command_sensor_plot(plot_contents_timestamps, plot_contents_right_command, plot_contents_right_sensor, False)