"""
EZLink application note for receiveing data
AN415

Hardie Pienaar 
Feb 2017
"""

import RFMLib as rfm
import time
import numpy as np

def setup():
    """Setup the library and turn on ism chip"""
    status = rfm.setup()
    """Read the interrupt status1 register"""
    ItStatus1 = rfm.read_register(0x03)
    ItStatus2 = rfm.read_register(0x04)

    """Set RF Parameters"""
    # Set the center frequency to 915MHz
    rfm.write_register(0x75, 0x75) # Write 0x75 to the Frequency Band Select register
    rfm.write_register(0x76, 0xBB) # Write 0xBB to the Nominal Carrier Frequency1 register
    rfm.write_register(0x77, 0x80) # Write 0x80 to the Nominal Carrier Frequency0 register
    # Configure the modem parameters for receiving the desired GFSK modulated data
    # (9.6 kbps 45 kHz deviation 112.1 kHz channel filter BW)
    rfm.write_register(0x1C, 0x05) # Write 0x05 to the IF Filter Bandwidth register
    rfm.write_register(0x20, 0xA1) # Write 0xA1 to the Clock Recovery Oversampling Ratio register
    rfm.write_register(0x21, 0x20) # Write 0x20 to the Clock Recovery Offset 2 register 
    rfm.write_register(0x22, 0x4E) # Write 0x4E to the Clock Recovery Offset 1 register 
    rfm.write_register(0x23, 0xA5) # Write 0xA5 to the Clock Recovery Offset 0 register 
    rfm.write_register(0x24, 0x00) # Write 0x00 to the Clock Recovery Timing Loop Gain 1 register 
    rfm.write_register(0x25, 0x13) # Write 0x13 to the Clock Recovery Timing Loop Gain 0 register 
    rfm.write_register(0x1D, 0x40) # Write 0x40 to the AFC Loop Gearshift Override register 
    rfm.write_register(0x72, 0x1F) # Write 0x1F to the Frequency Deviation register 

    """Configure the receive packet handler"""
    # Disable header bytes; set variable packet length (the length of the packet is defined by the
    # received packet length field of the packet); set the synch word to two bytes long
    rfm.write_register(0x33, 0x02) # Write 0x02 to Header Control 2 register 
    # Disable the receive header filters
    rfm.write_register(0x32, 0x00) # Write 0x00 to Header Control 1 register 
    # Disable the receive header filters
    rfm.write_register(0x32, 0x00) # Write 0x00 to Header Control 1 register 
    # Set the sync word pattern to 0x2DD4
    rfm.write_register(0x36, 0x2D) # Write 0x2D to the Sync Word 3 register
    rfm.write_register(0x37, 0xD4) # Write 0xD4 to the Sync Word 2 register
    # Enable the RX packet handler and CRC-16 (IBM) check
    rfm.write_register(0x30, 0x85) # Write 0x85 to the Data Access Control register
    # Enable FIFO mode and GFSK modulation
    rfm.write_register(0x71, 0x63) # Write 0x63 to the Modulation Mode Control 2 Register 
    # Set the preamble detection threshold to 20 bits 
    rfm.write_register(0x35, 0x28) # Write 0x30 to preamble detection control register 

    """Select modulation"""
    # Set VCO and PLL
    rfm.write_register(0x54, 0x7F) # Write 0x7F to the VCO Current Trimming register
    rfm.write_register(0x58, 0x80) # Write 0x80 to the Charge pump Current Trimming overide register 
    rfm.write_register(0x59, 0x40) # Write 0x40 to the Divider Current Trimming register
    # Set the AGC
    rfm.write_register(0x6A, 0x0B) # Write 0x0B to the AGC Overide 2 register 
    # Set the ADC reference voltage to 0.9V
    rfm.write_register(0x68, 0x04) # Write 0x04 to the Deltasigma ADC Tuning 2 register 
    rfm.write_register(0x1F, 0x03) # Write 0x03 to the Clock Recovery Gearshift Override register 

    """Enable receiver chain"""
    rfm.write_register(0x07, 0x05) # Write 0x05 to the Operating Function Control 1 register
    # Enable two interrupts
    # a) one which shows that a valid packet was received: 'ipkval'
    # b) second shows if the packet received with incorrect CRC: 'icrcerror'
    rfm.write_register(0x05, 0x03) # Write 0x03 to the Interrupt Enable 1 register
    rfm.write_register(0x06, 0x00) # Write 0x00 to the Interrupt Enable 2 register
    # Read interrupt status registers to clear pending interrupts making nIRQ pin go back to high
    ItStatus1 = rfm.read_register(0x03) # Read the Interrupt Status 1 register
    ItStatus2 = rfm.read_register(0x04) # Read the Interrupt Status 2 register

    return status

def receive_bytes():
   	# Receive packets 
	bytes_rec = 0

	while rfm.check_irq():
		time.sleep(0.000001)

	# Read interrupt status registers to clear pending interrupts making nIRQ pin go back to high
	ItStatus1 = rfm.read_register(0x03) # Read the Interrupt Status 1 register
	ItStatus2 = rfm.read_register(0x04) # Read the Interrupt Status 2 register

	"""CRC Error interrupt occured"""
	if ItStatus1 & 0x01 == 0x01:
		# Disable the receiver chain
		rfm.write_register(0x07, 0x01) # Write 0x01 to the Operating Function Control 1 register
		# Reset the RX FIFO
		rfm.write_register(0x08, 0x02) # Write 0x02 to the Operating Function Control 2 register
		rfm.write_register(0x08, 0x00) # Write 0x00 to the Operating Function Control 2 register
		print("CRC Error, FIFO flushed")
		# Enable the receiver chain
		rfm.write_register(0x07, 0x05) # Write 0x05 to the Operating Function Control 1 register

    
	payload = []
	if ItStatus1 & 0x02 == 0x02:
		# Disable the receiver chain
		rfm.write_register(0x07, 0x01) # Write 0x01 to the Operating Function Control 1 register
		# Read the length of the received payload
		length = rfm.read_register(0x4B)
		bytes_rec += length
		for i in np.arange(length):
			# Read all bytes into payload object
			payload.append(rfm.read_register(0x7F))
			# Read the length of the received payload
		#print(payload)
    
	# Read interrupt status registers to clear pending interrupts making nIRQ pin go back to high
	ItStatus1 = rfm.read_register(0x03) # Read the Interrupt Status 1 register
	ItStatus2 = rfm.read_register(0x04) # Read the Interrupt Status 2 register

	# Enable two interrupts again
	# a) one which shows that a valid packet was received: 'ipkval'
	# b) second shows if the packet received with incorrect CRC: 'icrcerror'
	rfm.write_register(0x05, 0x03) # Write 0x03 to the Interrupt Enable 1 register
	rfm.write_register(0x06, 0x00) # Write 0x00 to the Interrupt Enable 2 register

	# Read interrupt status registers to clear pending interrupts making nIRQ pin go back to high
	ItStatus1 = rfm.read_register(0x03) # Read the Interrupt Status 1 register
	ItStatus2 = rfm.read_register(0x04) # Read the Interrupt Status 2 register

	# Enable the receiver chain again
	rfm.write_register(0x07, 0x05) # Write 0x05 to the Operating Function Control 1 register
	
	return payload

def close():
	"""Cleanup GPIO and turn off the chip"""
	rfm.close()

