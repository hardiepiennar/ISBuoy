"""
Hardie Pienaar
Makespace
September 2019
"""

import binr
import serial
import os
import time
import numpy as np
import RPi.GPIO as GPIO
import EZLink_receive as rfm 
import subprocess

# Debug Switches
show_rover_msgs = False 
ignore_base = False 

# Serial port
port = "/dev/ttyAMA0"
nvs_speed = 115200
rfm_speed = 57600

# Pin connections
PIN_RST = 24    # NVS 
SPI_PORT = 0    # RFM
SPI_DEVICE = 0  # RFM

# Setting up GPIO port 
print("Setting up GPIO port")
GPIO.setmode(GPIO.BCM)
time.sleep(0.1) # Wait for things to settle
GPIO.setup(PIN_RST, GPIO.OUT)
time.sleep(0.1) # Wait for things to settle
GPIO.output(PIN_RST, GPIO.LOW)
time.sleep(0.1) # Wait for things to settle

# Connect and test telemetry radio
print("Connecting to telemetry radio")
rfm_connection = rfm.setup()
print(rfm_connection)
if rfm_connection:
	print("Telem radio communication success")
else:
	print("Telem radio communication fail")
	GPIO.cleanup()
	assert(False)

# Reset NVS 
print("Resetting NVS")
GPIO.output(PIN_RST, GPIO.HIGH)
time.sleep(0.1)
GPIO.output(PIN_RST, GPIO.HIGH)

# Connect to port and check for NMEA messages
print("Connecting to NVS serial")
ser_nvs = serial.Serial(port, nvs_speed)

# Wait for a GPGGA packet to verify serial port connection 
# Check ten messages
cnt = 0 
message_found = False
while cnt < 10 and not message_found:
	msg = ser_nvs.read(20)
	if "GPGGA" in str(msg):
		message_found = True
	cnt = cnt + 1
if message_found:
	print("Serial Connection Succesfull")
else:
	print("Serial Connection Failed")
	print("Cleaning up GPIO")
	GPIO.cleanup()
	print("Closing Serial")
	ser_nvs.close()

print("Switching serial protocol to BINR")
ser_nvs.write('$PORZA,0,115200,3*7E\r\n'.encode())
time.sleep(0.5)
ser_nvs.close()

time.sleep(1)
# Connect to device
print("Connecting to serial port [BINR "+str(nvs_speed)+"]")
ser_nvs = serial.Serial(port, nvs_speed, parity=serial.PARITY_ODD, timeout=0.5)
time.sleep(0.1)

#Configure NVS module
# Set navigation rate to 2 Hz
ser_nvs.write([0x10,0xD7,0x02,0x02,0x10,0x03])
time.sleep(0.2)
# Set raw data output intervals 5s:
ser_nvs.write([0x10,0xF4,0x32,0x10,0x03])
time.sleep(0.2)
# Turn on bit information transmitted by satellites
ser_nvs.write([0x10,0xD5,0x01,0x10,0x03])
time.sleep(0.2)
# Reboot without erasing saved parameters
#ser.write([0x10,0x01,0x00,0x01,0x21,0x01,0x00,0x01,0x10,0x03])
#time.sleep(0.2)

print("Requesting all ephemeris data")
ser_nvs.write([0x10,0x19,0xFF,0x01,0x10,0x03])
time.sleep(0.2)

# Request raw data
print("Requesting raw data")
ser_nvs.write(binr.request_raw_data(10))
time.sleep(0.2)

# Release serial port
print("Releasing NVS serial port for RTK process")
ser_nvs.close()

print("Killing any other socat process")
ports = os.listdir("/dev/pts/")
if len(ports) > 3:
	subprocess.call("pkill socat &", shell=True)
time.sleep(1)

print("Setting up proxy serial port..."),
old_ports = os.listdir("/dev/pts/")
subprocess.call("socat -d -d pty,raw,echo=0, pty,raw,echo=0 &", shell=True)
time.sleep(1)
print("Virtual port created")
new_ports = os.listdir("/dev/pts/")
for i in np.arange(len(old_ports)):
	new_ports.remove(old_ports[i])

# Open virtual port
print("Opening virtual port")
ser_vir = serial.Serial("/dev/pts/"+new_ports[0],rfm_speed,timeout=None)
#ser = serial.Serial("/dev/pts/"+new_ports[0],9600,parity=serial.PARITY_NONE,timeout=None)

# Start forwarding loop
print("Creating endless posting loop to port: "+"/dev/pts/"+new_ports[0])
print("Data available on: "+"/dev/pts/"+new_ports[1])
old_time = time.time()
try:
    while True:
        if not ignore_base:
            msg = rfm.receive_bytes()
            #print("msg: "+str(msg))
            ser_vir.write(bytearray(msg))

        if show_rover_msgs:
            try:
                buffer = list(ser_nvs.read_until(terminator='\x10\x03'))
                print("Waiting: "+str(ser_nvs.inWaiting()))
                if len(buffer) > 0:               
                    data, buffer = binr.process_msg(list(buffer))
                    print("Buffer: "+str(len(buffer)))
                    print("Msg: "+str(hex(data["ID"]))+": "+
                          str(len(data["data"]))+" bytes : "+
                          str(bytearray(data["data"][0:50])))
                    if data["ID"] == 0xF5:
                       raw_data = binr.process_raw_data(data["data"])
                       binr.print_raw_data(raw_data)
                time.sleep(0.1)
            except ValueError:
                    s = 1

except ValueError:
    print("Exception caught!")
finally:
    ser_vir.close()
    ser_nvs.close()
    rfm.close()
