import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import sys

# Configuration
PORT = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'
BAUD_RATE = 115200
MAX_POINTS = 100  # Number of points to display

# Data storage
timestamps = deque(maxlen=MAX_POINTS)
temp_data = deque(maxlen=MAX_POINTS)
press_data = deque(maxlen=MAX_POINTS)
hum_data = deque(maxlen=MAX_POINTS)

time_counter = 0

# Setup serial connection
try:
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"Connected to {PORT} at {BAUD_RATE} baud")
except Exception as e:
    print(f"Error opening serial port: {e}")
    sys.exit(1)

# Setup plot
fig, ax = plt.subplots(figsize=(12, 6))
fig.suptitle(f'Real-time Telemetry Data - {PORT}')

line_temp, = ax.plot([], [], 'r-', label='Temperature (°C)', linewidth=2)
line_press, = ax.plot([], [], 'b-', label='Pressure (hPa)', linewidth=2)
line_hum, = ax.plot([], [], 'g-', label='Humidity (%)', linewidth=2)

ax.set_ylabel('Value')
ax.set_xlabel('Sample')
ax.legend(loc='upper left')
ax.grid(True)
ax.set_ylim(0, 60)

def parse_data(line):
    """Parse data line: temp:33.5;press:1017.2;hum:50.9;"""
    data = {}
    try:
        parts = line.strip().split(';')
        for part in parts:
            if ':' in part:
                key, value = part.split(':')
                data[key] = float(value)
    except Exception as e:
        print(f"Parse error: {e}")
    return data

def animate(frame):
    global time_counter
    
    try:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"Received: {line}")
                data = parse_data(line)
                
                if 'temp' in data and 'press' in data and 'hum' in data:
                    timestamps.append(time_counter)
                    temp_data.append(data['temp'])
                    press_data.append(data['press'])
                    hum_data.append(data['hum'])
                    time_counter += 1
                    
                    # Update plots
                    line_temp.set_data(timestamps, temp_data)
                    line_press.set_data(timestamps, press_data)
                    line_hum.set_data(timestamps, hum_data)
                    
                    # Auto-scale axes
                    if len(timestamps) > 1:
                        ax.set_xlim(timestamps[0], timestamps[-1])
    
    except Exception as e:
        print(f"Error: {e}")
    
    return line_temp, line_press, line_hum

# Start animation
ani = animation.FuncAnimation(fig, animate, interval=100, blit=True, cache_frame_data=False)

plt.tight_layout()
plt.show()

# Cleanup
ser.close()
print("Serial port closed")