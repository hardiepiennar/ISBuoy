"""
This program creates a virtual serial port and forwards the data from the RFM22B radio to the 
created serial port.

Hardie Pienaar
Makespace Cambridge
June 2017
"""

import os
import subprocess
import serial
import serial.tools.list_ports
import EZLink_receive as rfm
import time
import numpy as np

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
ser = serial.Serial("/dev/pts/"+new_ports[0],57600,timeout=None)
#ser = serial.Serial("/dev/pts/"+new_ports[0],9600,parity=serial.PARITY_NONE,timeout=None)

# Create RFM22B receive link object
print("Setting up RFM22B")
rfm.setup()

# Start forwarding loop
print("Creating endless posting loop to port: "+"/dev/pts/"+new_ports[0])
print("Data available on: "+"/dev/pts/"+new_ports[1])
try:
	while True:
		msg = rfm.receive_bytes()
		#print("msg: "+str(msg))
		ser.write(bytearray(msg))
except:
	print("Exception caught!")
finally:
	ser.close()
	rfm.close()
