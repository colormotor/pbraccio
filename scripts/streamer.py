#%%
# Requires installing cobs https://pypi.org/project/cobs/
import serial
import time
import struct
from cobs import cobs
import numpy as np
import pickle
import os

def load_pkl(data_path):
    with open(os.path.expanduser(data_path), "rb") as f:
        data = pickle.load(f)
    return data


# Configure the serial port
port='/dev/cu.usbmodem101'
baud_rate = 9600  # Match the Arduino's baud rate
Q = load_pkl('~/tmp/Q.pkl')
#Q = Q + Q[::-1]
print(Q)
#%%
def degrees( x ):
    return x * (180.0/np.pi)

# Configure the serial port. You may need to adjust the port name and baud rate.
ser = serial.Serial(port, 9600, timeout=0.1)  # Replace 'COM1' with your Arduino's serial port and adjust the baud rate.

start = time.time()

i = 0
n = len(Q)
try:
    for i in range(n):
        t = time.time() - start
        # Generate 5 random integers as an example.
        data = (np.ones(6)*90).astype(float)
        #data[2] = 90 + np.sin(t)*30.0
        #data[-1] = 90 + np.sin(t*2)*90
        
        ii = i%n

        # if ii > (n - 100) or ii < 5:
        #     data[-1] = 100 #-90
        # else:
        #     data[-1] = 0

        for j in range(5):
            data[j] = max(0.0, degrees(Q[ii][j]))

        # Pack the data into a binary format (4 bytes per integer).
        packet = struct.pack('6f', *data)
        packet = cobs.encode(packet)
        # Send the packet to the Arduino.
        ser.write(packet + b'\x00')
        ser.flush()  # Flush the serial buffer to ensure data is sent immediately.


        # print(f"Sent: {data}")

        time.sleep(0.001)  # Adjust the delay as needed to control the sending rate.

except KeyboardInterrupt:
    print("Interrupted")

finally:
    ser.close()
    
# %%
