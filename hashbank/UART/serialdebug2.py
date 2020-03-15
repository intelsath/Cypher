import serial
import time

serial = serial.Serial("COM2", 115200, timeout=1)
serial.write("PC_HNDSHK")
