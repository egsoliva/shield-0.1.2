import serial
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
import time

# Change serial port to the port used in Arduino IDE
ser = serial.Serial('COM13', 115200, timeout=1)
plt.ion()  # Interactive mode for real-time plotting

fig, ax = plt.subplots(3, 1, figsize=(10, 8))
fig.suptitle('ADXL345 Accelerometer Data')

max_samples = 200
time_buffer = deque(maxlen=max_samples)
x_buffer = deque(maxlen=max_samples)
y_buffer = deque(maxlen=max_samples)
z_buffer = deque(maxlen=max_samples)

full_data = []
start_time = time.time()

try:
    while time.time() - start_time <= 30: 
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            if line:
                try:
                    t, x, y, z = map(float, line.split(','))
                    time_buffer.append(t)
                    x_buffer.append(x)
                    y_buffer.append(y)
                    z_buffer.append(z)
                    full_data.append([t, x, y, z])

                    ax[0].clear()
                    ax[0].plot(time_buffer, x_buffer, 'r', label='X-axis')
                    ax[0].set_ylabel('Accel (X)')
                    ax[0].legend()
                    
                    ax[1].clear()
                    ax[1].plot(time_buffer, y_buffer, 'g', label='Y-axis')
                    ax[1].set_ylabel('Accel (Y)')
                    ax[1].legend()
                    
                    ax[2].clear()
                    ax[2].plot(time_buffer, z_buffer, 'b', label='Z-axis')
                    ax[2].set_ylabel('Accel (Z)')
                    ax[2].set_xlabel('Time (ms)')
                    ax[2].legend()
                    
                    plt.pause(0.01)  
                
                except ValueError:
                    pass  

    ser.close()
    print("30 seconds elapsed. Serial connection closed.")

    '''
    # Uncomment this if you want to log data in excel (csv)
    full_data = np.array(full_data)
    np.savetxt('accel_data_30s_v2.csv', full_data, 
               delimiter=',', header='Time(ms),X,Y,Z', comments='')
    print("Data saved to accel_data_30s_v2.csv")
    '''
    
    plt.show(block=True)

except KeyboardInterrupt:
    ser.close()
    print("Serial connection closed.")
    plt.show(block=True) 