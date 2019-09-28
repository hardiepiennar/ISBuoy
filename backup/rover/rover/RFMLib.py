"""
RFM device library
H Pienaar Dec 2016
"""

import RPi.GPIO as GPIO
import spidev
import time
import numpy as np

#Pin definitions
PIN_SDN = 26 
PIN_NIRQ = 6 

#Things that could be done
#Add function to set microcontroller output clock
#Add function to adjust crystal load capacitance 
#Add GPIO configuration function 
#Add ADC function
#Add Temperature functions
#Add Wake up timer functions 
#Add Low Duty Cycle Configuration
#Add Low Battery Functions 
#Add AFC and Clock recovery functions
#Add Antenna Diversity functions

#TODO:
#IF filter bandwidth function
#Add RSSI function
#Implement registers after 0x30

#IF Filter lookup table
IFFilter = [2600,2800,3100,3200,3700,4200,4500,4900,5400,5900,6100,7200,8200,8800,9500,10600,11500,12100,14200,16200,17500,18900,21000,22700,24000,28200,32200,34700,37700,41700,45200,47900,56200,64100,69200,75200,83200,90000,95300,112100,127900,137900,142800,167800,181100,191500,225100,248800,269300,284900,335500,361800,420200,469400,518800,577000,620700]
IFFilterNDecExp = [5,5,5,5,5,5,5,4,4,4,4,4,4,4,3,3,3,3,3,3,3,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0]
IFFilterDwn3Bypass = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
IFFilterFilset = [1,2,3,4,5,6,7,1,2,3,4,5,6,7,1,2,3,4,5,6,7,1,2,3,4,5,6,7,1,2,3,4,5,6,7,1,2,3,4,5,6,7,4,5,9,15,1,2,3,4,8,9,10,11,12,13,14]

spi = spidev.SpiDev()

def setup():
    print("Setting up RFM22B GPIO connections..."),
    #Setup GPIO pins
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(PIN_SDN, GPIO.OUT)
    GPIO.setup(PIN_NIRQ, GPIO.IN)

    #Initialize output pins
    GPIO.output(PIN_SDN, GPIO.HIGH)
    print("[DONE]")

    #Turn on RFM chip
    print("Turning on RFM22B..."),
    GPIO.output(PIN_SDN, GPIO.LOW)
    time.sleep(0.015)
    print("[DONE]")

    #Initialize spi library
    bus = 0 
    device = 0
    spi.open(bus,device)
    spi.max_speed_hz = 20000000
    time.sleep(0.1)

    print("Checking communication with RFM chip"),
    if check_communication():
        print("[Success]")
        return True
    else:
        print("[Fail]")
        return False

def read_register(addr):
    read = spi.xfer2([addr,0x00])[1]
    return read 

def write_register(addr, byte):
    spi.xfer2([addr|0b10000000, byte])

def close():
    print("Turning off RFM22B..."),
    GPIO.output(PIN_SDN, GPIO.HIGH)
    print("[DONE]")

def check_irq():
    return GPIO.input(PIN_NIRQ)

def print_current_mode():
    """Reads register 0x07 and prints out the current mode"""
    mode_reg = read_register(0x07)
    print("\n======Operating and Function Control======")

    print("Crystal Oscillator (xton): "),
    if mode_reg&0b00000001 > 0:
        print("\t[ON]")
    else:
        print("\t[OFF]")

    print("Phase Locked Loop (pllon): "),
    if mode_reg&0b00000010 > 0:
        print("\t[ON]")
    else:
        print("\t[OFF]")

    print("Receive Mode (rxon): "),
    if mode_reg&0b00000100 > 0:
        print("\t\t[ON]")
    else:
        print("\t\t[OFF]")

    print("Transmit Mode (txon): "),
    if mode_reg&0b00001000 > 0:
        print("\t\t[ON]")
    else:
        print("\t\t[OFF]")

    print("32 kHz Clock Source (32ksel): "),
    if mode_reg&0b00010000 > 0:
        print("\t[ON]")
    else:
        print("\t[OFF]")

    print("Watchdog Timer (enwt): "),
    if mode_reg&0b00100000 > 0:
        print("\t[Enabled]")
    else:
        print("\t[Disabled]")

    print("Low Battery Detect (enlbd): "),
    if mode_reg&0b01000000 > 0:
        print("\t[Enabled]")
    else:
        print("\t[Disabled]")

    mode_reg = read_register(0x08)
    print("Auto Transmit (autotx): "),
    if mode_reg&0b00001000 > 0:
        print("\t[Enabled]")
    else:
        print("\t[Disabled]")

    print("Multi-packet Receive (rxmpk): "),
    if mode_reg&0b00010000 > 0:
        print("\t[Enabled]")
    else:
        print("\t[Disabled]")

    print("Ant diversity mode (antdiv): "),
    print("\t["+str(mode_reg&0b11100000/32)+"]")

    print("==========================================\n")

def soft_reset():
    #TODO: use register 0x07 to reset by setting swres
    print("ERROR: soft_reset function not yet implemented")

def clear_tx_fifo():
    #TODO: use register 0x08 to clear tx fifo by setting ffclrtx 
    print("ERROR: clear_tx_fifo function not yet implemented")

def clear_rx_fifo():
    #TODO: use register 0x08 to clear rx fifo by setting ffclrrx 
    print("ERROR: clear_rx_fifo function not yet implemented")

def set_auto_tx(state):
    #TODO: use register 0x08 to turn on or off autotx 
    print("ERROR: set_auto_tx function not yet implemented")

def check_communication():
    """Read register 0x00 and make sure it returns 0x07"""
    test_value = read_register(0x01)
    print("SPI returned: "+str(test_value))	
    if test_value == 0x06:
        return True
    else:
        return False

def print_int_status():
    """Prints the status of all of the interrupts and if they are enabled or not"""

    int_stat_1 = read_register(0x03)
    int_stat_2 = read_register(0x04)
    int_en_1 = read_register(0x05)
    int_en_2 = read_register(0x06)

    def print_en_trig(name, en_comp, trig_comp):
        print(name+" "),
        if(en_comp > 0):
            print("[Enabled] "),
        else:
            print("[Disabled]"),
        print(":"),
        if(trig_comp > 0):
            print("[Triggered]")
        else:
            print("[Off]")

    print("\n================Interrupt Status================")
    print_en_trig("CRC Error            ", int_en_1 & 0b00000001, int_stat_1 & 0b00000001)
    print_en_trig("Valid Packet         ", int_en_1 & 0b00000010, int_stat_1 & 0b00000010)
    print_en_trig("Packet Sent          ", int_en_1 & 0b00000100, int_stat_1 & 0b00000100)
    print_en_trig("External             ", int_en_1 & 0b00001000, int_stat_1 & 0b00001000)
    print_en_trig("RX FIFO Almost Full  ", int_en_1 & 0b00010000, int_stat_1 & 0b00010000)
    print_en_trig("TX FIFO Almost Empty ", int_en_1 & 0b00100000, int_stat_1 & 0b00100000)
    print_en_trig("TX FIFO Almost Full  ", int_en_1 & 0b01000000, int_stat_1 & 0b01000000)
    print_en_trig("FIFO Error           ", int_en_1 & 0b10000000, int_stat_1 & 0b10000000)

    print_en_trig("Power On Reset       ", int_en_2 & 0b00000001, int_stat_2 & 0b00000001)
    print_en_trig("Chip Ready           ", int_en_2 & 0b00000010, int_stat_2 & 0b00000010)
    print_en_trig("Low Battery Detect   ", int_en_2 & 0b00000100, int_stat_2 & 0b00000100)
    print_en_trig("Wake Up Timer        ", int_en_2 & 0b00001000, int_stat_2 & 0b00001000)
    print_en_trig("RSSI                 ", int_en_2 & 0b00010000, int_stat_2 & 0b00010000)
    print_en_trig("Invalid Preamble     ", int_en_2 & 0b00100000, int_stat_2 & 0b00100000)
    print_en_trig("Valid Preamble       ", int_en_2 & 0b01000000, int_stat_2 & 0b01000000)
    print_en_trig("Sync Word Detected   ", int_en_2 & 0b10000000, int_stat_2 & 0b10000000)
    print("===============================================\n")

def print_dev_status():
    """Prints out the status register of the device"""
    dev_status = read_register(0x02)
    print("\n============Device Status============")
    print("Chip Power State:\t"),
    if(dev_status&0b00000011 == 0):
        print("[Idle]")
    elif(dev_status&0b00000011 == 1):
        print("[RX]")
    else:
        print("[TX]")
    print("Header Error:\t\t"),
    if(dev_status&0b00010000 > 0):
        print("[True]")
    else:
        print("[False]")
    print("RX FIFO Empty:\t\t"),
    if(dev_status&0b00100000 > 0):
        print("[True]")
    else:
        print("[False]")
    print("RX/TX FIFO Underflow:\t"),
    if(dev_status&0b01000000 > 0):
        print("[True]")
    else:
        print("[False]")
    print("RX/TX FIFO Overflow:\t"),
    if(dev_status&0b10000000 > 0):
        print("[True]")
    else:
        print("[False]")
    print("=====================================\n")

def print_ezmac_status():
    """Prints out the ezmac status register of the device"""
    dev_status = read_register(0x31)
    print("\n============EZMAC Status============")
    print("Packet Sent:\t\t"),
    if(dev_status&0b00000001 > 0):
        print("[True]")
    else:
        print("[False]")
    print("Packet Transmitting:\t"),
    if(dev_status&0b00000010 > 0):
        print("[True]")
    else:
        print("[False]")
    print("CRC Error:\t\t"),
    if(dev_status&0b00000100 > 0):
        print("[True]")
    else:
        print("[False]")
    print("Valid Packet Received:\t"),
    if(dev_status&0b00001000 > 0):
        print("[True]")
    else:
        print("[False]")
    print("Packet Receiving:\t"),
    if(dev_status&0b00010000 > 0):
        print("[True]")
    else:
        print("[False]")
    print("====================================\n")

def get_rssi():
    """Reads the RSSI register and returns the value in dBm"""
    rssi_byte = read_register(0x26)
    rssi = (10/19)*rssi_byte - 126.32 
    return rssi 

def set_freq(freq):
    """
    Sets up the RFM22B registers to listen or transmit at the given frequency
    Returns True if the command was succesfull
    freq is specified in MHz
    """
    #Check if frequency is in range
    if(freq > 960 or freq < 240):
        return False

    #Some variables that will be used
    bandSelect = 0b01000000
    fb = 0b00000000
    fc = 0
    xtalFreq = 30000
                        
    #Calculate register according to formula found in excel sheet
    if(freq >= 480):
        bandSelect |= 0b00100000
        temp = freq/(10*(xtalFreq/30000)*(1+1))
    else:
        temp = freq/(10*(xtalFreq/30000)*(1+0))
        fb = int(np.floor(temp)) - 24
        fc = int(np.floor((temp - np.floor(temp))*64000+0.49999))
                                                                                            
    bandSelect |= fb
    fc1 = fc>>8
    fc2 = fc&0b0000000011111111
                                                                                                            
    #   print(bandSelect)
    #   print(fc1)
    #   print(fc2)
    #Write the calculated registers to the device
    write_register(0x75,bandSelect)
    write_register(0x76,fc1)
    write_register(0x77,fc2)

    #print("Freq Register "+str(readRFM22BRegister(spi,0x76)))
                                                                                                                            
    return True
   
def set_if_filter(rbw):
    """
    Sets the IF filter register for the specified bandwidth by using a lookup table
    rbw is the bandwidth in Hz
    returns True if filter was found in table and set
    """
    #Run through list to find IF chosen index
    index = -1
    for i in np.arange(0,len(IFFilter)):
        if(int(rbw) == int(IFFilter[i])):
            index = i

    #Return Flase if rbw was not found in list
    if index < 0:
        print("ERROR: Filter value not found in lookup table")
    return False

    #Construct registers out of lookup table
    ifReg = 0b00000000
    ifReg |= (IFFilterDwn3Bypass[index]<<7)
    ifReg |= (IFFilterNDecExp[index]<<4)
    ifReg |= (IFFilterFilset[index])

    #Write registers to RFM22B
    write_register(0x1C,ifReg)

    #Check
    #print("IF Register "+str(readRFM22BRegister(spi,0x1C)))
    return True     

def set_tx_mode():
    """Sets the RFM into tx mode"""
    reg = read_register(0x07)
    write_register(0x07, reg|0b00001000)

def set_rx_mode():
    """Sets the RFM into rx mode"""
    reg = read_register(0x07)
    write_register(0x07, reg|0b00000100)

def turn_on_pll():
    """Turns on the PLL"""
    reg = read_register(0x07)
    write_register(0x07, reg|0b00000010)

def set_fifo_mode():
    """Sets the data modulation source to FIFO"""
    reg = read_register(0x71)
    write_register(0x71, reg|0b00100000)
    write_register(0x71, reg&0b11101111)

def write_fifo_data(data):
    """Writes a data to the fifo"""
    write_register(0x7F, data) #TODO: add support for burst write

if __name__ == "__main__":
    setup()
    if(check_communication()):
        print("RFM22B Detected")
        print(read_register(0x01))
        close()
        exit() 
        print_current_mode()
        #print_int_status()
        print_dev_status()
        print_ezmac_status()
        
        print("Setting frequency..."),
        if set_freq(868):
            print("[Done]")
        else:
            print("[Failed]")

        print("Filling up FIFO with bytes"),

        for i in np.arange(65): 
            print("."),
            write_fifo_data(i)

        print("[DONE]")

        print_dev_status()

        print("Turn on PLL"),
        turn_on_pll()
        print("[Done]")

        print("Set transmit mode..."),
        set_tx_mode()
        print("[Done]")

        time.sleep(0.5)
        print_current_mode()
        print_dev_status()
        print_ezmac_status()

        
    else:
        print("RFM22B Communication Failed")
    close()
    GPIO.cleanup()
