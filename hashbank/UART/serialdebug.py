import serial
import time

serial = serial.Serial("COM2", 115200, timeout=1)
# Read
while 1:
    n_bytes = 9
    data = serial.read(n_bytes)
    print 'Read: ' + data
