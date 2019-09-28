"""
NVS BINR protocol functions.

Hardie Pienaar
Bosmans Crossing
June 2018
"""

import struct
import numpy as np

# Global variables
WGS84 = 0
PZ90 = 1
SK42 = 2
SK95 = 3
PZ90_2 = 4
GPS = 1
GLONASS = 2


def process_msg(buffer):
    """
    Process a BINR message and return a dictionary with 
    # the message ID and data.

    <DLE><ID>[data]<DLE><ETX>
    
    arguments:
        buffer - byte buffer containing BINR data
    returns:
        {message_ID, data}
            data - array of data bytes contained in the message
        remaining_buffer - buffer with processed msg removed
    raises:
        ValueError - If no message could be found
    """
    # Check that message has valid start and stop bytes
    # and grab the message string from the buffer
    try:
        start_id = buffer.index(0x10)
        searching = True
        stop_id_offset = 0
        while searching:
            stop_id = buffer[start_id+1+stop_id_offset:].index(0x10)+start_id+1+stop_id_offset
            if len(buffer) >= stop_id + 2 and buffer[stop_id + 1]== 0x03:
                searching = False
            else:
                stop_id_offset = stop_id+1
        msg = buffer[start_id:stop_id+2]
    except ValueError:
        raise ValueError("Buffer did not contain message")

    # Grab the message id and data segments
    message_id = struct.unpack('<B',bytearray(msg[1:1+1]))[0] # Flag of user dynamic 0 - static, 1 - kinematic

    data = msg[2:-2]

    # Return the message and rest of the buffer
    return {"ID":message_id,"data":data}, buffer[stop_id+2:]

def reboot(erase=False):
    """
    Reboot device packet

    arguments:
        erase - False - erase saved system data and user settings
                True  - without erasing all saved parameters
    """
    erase_value = 0x01
    if erase:
        erase_value = 0x00
        
    # Generate packet
    packet = [0x10,0x01,
              0x00, 0x01, 0x21, 0x01, 0x00, erase_value,
              0x10, 0x03]
    return packet

def set_coordinate_system(coordinate_system):
    """
    Sets the coordinate system used in the system.

    argument:
        coordinate_system - 00 WGS-84
                            01 PZ-90
                            02 Pulkovo 42 Coordinate System (SK42)
                            03 Pulkovo 95 Coordinate System (SK95)
                            04 PZ-90.02
    """
    # Check for valid input
    if coordinate_system > 4 or coordinate_system < 0:
        raise ValueError("Not a valid coordinate system: "+str(coordinate_system))

    # Generate packet
    packet = [0x10, 0x0D,
              0x01, coordinate_system,
              0x10, 0x03]
    return packet

def set_navigation_system(navigation_system):
    """
    Set the constellation used for navigation

    arguments:
        navigation_system = 0 GPS,GLONASS
                            1 GPS
                            2 GLONASS
                            3 GALILEO
                            10 GPS, GLONASS, SBAS
                            11 GPS, SBAS
                            12 GLONASS, SBAS                            
    """
    if (navigation_system < 0 or \
        (navigation_system > 3 and navigation_system < 10) or\
        navigation_system > 12):
        raise ValueError("Invalid navigation system: "+str(navigation_system))
    # Generate packet
    packet = [0x10, 0x0D,
              0x02, navigation_system,
              0x10, 0x03]
    return packet

def set_pvt_setting(min_sat_elev_mask, min_snr, max_rms_error):
    """
    Set the PVT settings

    arguments:
        min_sat_elev_mask - Value of the minimum satellite elevation mask for 
                            navigation use [deg] [0-90]
        min_snr - The minimum signal to noise ratio for use in navigation [0-39]                       
        max_rms_error - The maximum rms error at which the navigation task shall
                        be deemed valid. [meters]
    """
    # Test elevation mask input
    if min_sat_elev_mask < 0 or min_sat_elev_mask > 90:
        raise ValueError("Invalid elevation mask: "+str(min_sat_elev_mask))
    if min_snr < 0 or min_snr > 39:
        raise ValueError("Invalid min snr: "+str(min_snr))
    if max_rms_error < 0:
        raise ValueError("Invalid max rms error: "+str(max_rms_error))
    # Generate packet
    packet = [0x10, 0x0D,
              0x03, min_sat_elev_mask, min_snr, max_rms_error&0x00FF,(max_rms_error&0xFF00)/256,
              0x10, 0x03]
    return packet

def set_filtration_factor(factor):
    """
    Sets the filtration factor of the coordinates.

    arguments:
        factor - filtration factor [0-10]
    """
    if factor < 0 or factor > 10:
        raise ValueError("Invalid filtration factor: "+str(factor))
    packet = [0x10, 0x0D,
              0x04, 0x00, 0x00, 0x00, factor*2*16,
              0x10, 0x03]
    return packet

def cancel_requests():
    """
    Clears the list of output messages
    """
    return [0x10,0x0E,0x10,0x03]
    
def set_ref_coordinates(lat, lon, height):
    """
    Set the reference antenna coodinates for fixed coordinate operating mode
    
    arguments:
        lat - latitude [rad]
        lon - longitude [rad]
        height - height [m]
    """
   
    # Convert the inputs to byte arrays
    lat_hex = struct.pack('<d',lat)
    lon_hex = struct.pack('<d',lon)
    height = struct.pack('<d',height)
    
    # Create packet
    packet = [0x10, 0x0F, 0x03]
    packet = packet + list(lat_hex)
    packet = packet + list(lon_hex)
    packet = packet + list(height)
    packet = packet + [0x10, 0x03]
    return packet

def enable_sat(sat_system, sat_no, state=True):
    """
    Enabling or disabling the use of specific satellites for navigation.

    arguments:
        sat_system - GPS or GLONASS
        sat_no     - satellite ID (1-32 for GPS or 1-24 for GLONASS)
        state      - True to enable the satellite, False to disable
    """ 
    # Test for bad arguments
    if sat_system != GPS and sat_system != GLONASS:
        raise ValueError("Satellite system invalid: "+str(sat_system))
    if sat_system == GPS:
        if sat_no < 1 or sat_no > 32:
            raise ValueError("SPS satellite number not valid: "+str(sat_no))
    else:
        if sat_no < 1 or sat_no > 24:
            raise ValueError("Glonass satellite number not valid: "+str(sat_no))

    # Create packet
    if state:
        state_packet = 1
    else:
        state_packet = 2
    packet = [0x10,0x12,
              sat_system, sat_no, state_packet, 
              0x10, 0x03]
    return packet

def request_status_of_receiver_channels():
    """
    Generates a packet to request the current status of the receiver channels

    Responds with message 42
    """
    return [0x10, 0x17, 0x10, 0x03]

def process_status_of_receiver_channels(data):
    """
    Process the response of the status of receiver channel request.

    arguments:
        data - data portion of response message
    
    returns:
        A dictionary is returned with the following format:
        list[{System, Number, SNR, Status, Pseudorange Sign}]

        Every list item reperesents a single channel.

        System - 1 GPS, 2 GLONASS, 4, SBAS, 8 - Galileo E1b
        Number - 1-32 GPS, 120-138 SBAS, -7-6 GLONASS
        SNR - Signal to noise ratio
        Status - 0-Automatic, 1-Manual, 2-Being Tested, 3-Error
        Pseudorange sign - 0-Digitization and measurement of pseudorange, 
                           1-Failure,
                           2-Pseudorange measurement available, digitization not 
    """
    # Test if packet is valid using num channels not a float property

    # Get the number of channels from the length of the data
    num_channels = int(len(data)/6)
    
    # Parse receiver data
    receiver_channels = []
    for i in range(num_channels):
        system = data[0+i*5]
        number = data[1+i*5]
        if system == 2: # If GLONASS, convert to signed int
            if number > 128:
                number = (256-number)*-1        
        snr = data[2+i*5]
        ch_status = data[3+i*5]
        pseudo_status = data[4+i*5]
        receiver_channels.append({"System":system,
                                  "Number":number,
                                  "SNR":snr,
                                  "Status":ch_status,
                                  "Pseudorange Sign":pseudo_status})
    return receiver_channels

def print_status_of_receiver_channels(channel_status):
    """
    Print the processed result of the status of receiver channel request.
    """
    for i in range(len(channel_status)):
        if channel_status[i]["System"] == 1:
            sys = "GPS"
        elif channel_status[i]["System"] == 2:
            sys = "GLO"
        elif channel_status[i]["System"] == 4:
            sys = "SBAS"
        elif channel_status[i]["System"] == 8:
            sys = "Galileo E1b"
        else:
            sys = "None"
        
        if channel_status[i]["Status"] == 0:
            stat = "Auto"
        elif channel_status[i]["Status"] == 1:
            stat = "Manual"
        elif channel_status[i]["Status"] == 2:
            stat = "Testing"
        elif channel_status[i]["Status"] == 3:
            stat = "Error"
        else:
            stat = "None"

        if channel_status[i]["Pseudorange Sign"] == 0:
            pseudo = "Pseudorange Digitization and Measurement"
        elif channel_status[i]["Pseudorange Sign"] == 1:
            pseudo = "Failure"
        elif channel_status[i]["Pseudorange Sign"] == 2:
            pseudo = "Pseudorange Measurement"
        else:
            pseudo = "None"

        print("Channel: "+str(i)+": "+str(sys)+"("\
            +str(channel_status[i]["Number"])+")\tSNR: "\
            +str(channel_status[i]["SNR"])+"\t"\
            +str(stat)+"\t"\
            +str(pseudo))

def request_sv_ephemeris(sat_system, sat_no, carrier=None):
    """
    Requests the ephemerides of the specified satellite.

    arguments:
        sat_system - 1-GPS, 2-GLONASS
        sat_no - 1-32(GPS), 1-24(GLONASS)
        carrier - GLONASS Carrier frequency number
    """
    # Test input values
    if sat_system != GPS and sat_system != GLONASS:
        raise ValueError("Satellite system invalid: "+str(sat_system))
    if sat_system == GPS:
        if sat_no < 1 or sat_no > 32:
            raise ValueError("SPS satellite number not valid: "+str(sat_no))
    else:
        if sat_no < 0 or sat_no > 24:
            raise ValueError("Glonass satellite number not valid: "+str(sat_no))
    # TODO: create carrier check

    # Create package
    packet = [0x10, 0x19, sat_system, sat_no]
    if carrier != None:
        packet = packet + [(256-(carrier*-1))]
    packet = packet + [0x10, 0x03]

    return packet

def process_sv_ephemeris(data):
    """
    Process the ephemeres packet (message 0x49) data and return a 
    dictionary containing the different parameters.

    arguments:
        data - data portion of response message
    ]
    returns:
        A dictionary is returned with the format shown in the BINR 
        Protocol specification ver 1.3 page 46.
    """

    # Grab and unpack data
    sat_system = data[0]
    if sat_system == GPS:
        prn = data[1] # On-booard number
        c_rs = struct.unpack('<f',bytearray(data[2:2+4]))[0] # Orbit Radius Sine Correction [m]
        dn = struct.unpack('<f',bytearray(data[6:6+4]))[0] # Difference between principle motion and defined motion [rad/ms]
        m_0 = struct.unpack('<d',bytearray(data[10:10+8]))[0] # Mean Anomaly[rad]
        c_uc = struct.unpack('<f',bytearray(data[18:18+4]))[0] # Longitude Argument Cosine Correction [rad]
        e = struct.unpack('<d',bytearray(data[22:22+8]))[0] # Eccentricity
        c_us = struct.unpack('<f',bytearray(data[30:30+4]))[0] # Latitude Argument Sine Correction [rad]
        sqrtA = struct.unpack('<d',bytearray(data[34:34+8]))[0] # Square root of the major semi-axis [sqrt_m]
        t_0e = struct.unpack('<d',bytearray(data[42:42+8]))[0] # Ephemerides reference time [ms]
        c_ic = struct.unpack('<f',bytearray(data[50:50+4]))[0] # Cosine correction to the inclination angle [rad]
        omega_0 = struct.unpack('<d',bytearray(data[54:54+8]))[0] # Orbital Plane Ascending Node Longitude [rad]
        c_is = struct.unpack('<f',bytearray(data[62:62+4]))[0] # Sine correction to the inclination angle [rad]
        i_0 = struct.unpack('<d',bytearray(data[66:66+8]))[0] # Inclination angle [rad]
        c_rc = struct.unpack('<f',bytearray(data[74:74+4]))[0] # Orbit Radius Cosine Correction [m]
        w = struct.unpack('<d',bytearray(data[78:78+8]))[0] # Ascending node-perigee angle [rad]
        omega_dot = struct.unpack('<d',bytearray(data[86:86+8]))[0] # Direct Descending Change Speed [rad/ms]
        idot = struct.unpack('<d',bytearray(data[94:94+8]))[0] # Inclination angle change speed [rad/ms]
        t_gd = struct.unpack('<f',bytearray(data[102:102+4]))[0] # Group Differential Delay Estimation [ms]
        t_0c = struct.unpack('<d',bytearray(data[106:106+8]))[0] # Time correction [ms]
        a_f2 = struct.unpack('<f',bytearray(data[114:114+4]))[0] # Time correction [ms/ms^2]
        a_f1 = struct.unpack('<f',bytearray(data[118:118+4]))[0] # Time correction [ms/ms^2]
        a_f0 = struct.unpack('<f',bytearray(data[122:122+4]))[0] # Time correction [ms]
        ura = struct.unpack('<H',bytearray(data[126:126+2]))[0] # User measurement accuracy
        iode = struct.unpack('<H',bytearray(data[128:128+2]))[0] # An identifier of a setofephemerides

        return {"System":sat_system, "PRN":prn, "C_rs":c_rs, "dn":dn, "M_0":m_0,
                "C_uc":c_uc, "e":e,"C_us":c_us,"sqrtA":sqrtA, "t_0e":t_0e,
                "C_ic":c_ic, "Omega_0":omega_0, "C_is":c_is, "I_0":i_0, 
                "C_rc":c_rc, "w":w, "Omega_dot":omega_dot, "IDOT":idot,
                "T_GD":t_gd, "t_0c":t_0c, "a_f2":a_f2, "a_f1":a_f1, 
                "a_f0":a_f0, "URA":ura, "IODE":iode}
    elif sat_system == GLONASS:
        nA = struct.unpack('<B',bytearray(data[1:1+1]))[0] # On-board Number
        HnA = struct.unpack('<b',bytearray(data[2:2+1]))[0] # Carrier frequency number
        xn = struct.unpack('<d',bytearray(data[3:3+8]))[0] # Coordinates [m]
        yn = struct.unpack('<d',bytearray(data[11:11+8]))[0] # Coordinates [m]
        zn = struct.unpack('<d',bytearray(data[19:19+8]))[0] # Coordinates [m]
        xnv = struct.unpack('<d',bytearray(data[27:27+8]))[0] # Speed [m/ms]
        ynv = struct.unpack('<d',bytearray(data[35:35+8]))[0] # Speed [m/ms]
        znv = struct.unpack('<d',bytearray(data[43:43+8]))[0] # Speed [m/ms]
        xna = struct.unpack('<d',bytearray(data[51:51+8]))[0] # acceleration [m/ms^2]
        yna = struct.unpack('<d',bytearray(data[59:59+8]))[0] # acceleration [m/ms^2]
        zna = struct.unpack('<d',bytearray(data[67:67+8]))[0] # acceleration [m/ms^2]
        tb = struct.unpack('<d',bytearray(data[75:75+8]))[0] # Time interval inside the current day [msec]
        gamma_n = struct.unpack('<f',bytearray(data[83:83+4]))[0] # Signal-carrier frequency value relative deviation
        tau_n = struct.unpack('<f',bytearray(data[87:87+4]))[0] # Satellite time scale offset value in relation to the GLONASS scale [ms]
        e_n = struct.unpack('<H',bytearray(data[91:91+2]))[0] # Satellite time scale offset value in relation to the GLONASS scale [ms]
  
        return {"System":sat_system, "n^A":nA, "H_n^A":HnA,
                "x_n":xn, "y_n":yn, "z_n":zn, 
                "x_nv":xnv, "y_nv":ynv, "z_nv":znv,
                "x_na":xna, "y_na":yna, "z_na":zna,
                "t_b":tb, "gamma_n":gamma_n, "tau_n":tau_n,
                "E_n":e_n}
                
def request_raw_data(measurement_interval=10):
    """
    Request raw data output (message F5h) at a set interval
    
    arguments:
        measurement_interval - 100ms intervals between messages
    
    returns:
        generated packet
    """
    if measurement_interval < 1:
        raise ValueError("Measurement interval should be > 0")

    # Create package
    packet = [0x10, 0xF4, 
              measurement_interval, 
              0x10, 0x03]

    return packet

def process_raw_data(data):
    """
    Process the Raw data package F4h and return a dictionary 
    object with the data.

    arguments:
        data - packet data

    return:
        dictionary with data fields described in BINR Protocol v1.3 
        page 69.
    """
    # Main 28 bytes
    offset = 0
    value, slip = read_double(data,offset, min=0, max=1E10, allow_negative=False, tries=5)
    offset = offset + 8 + slip
    tm = value # Time of measurements, UTC [ms] 

    week_num = struct.unpack('<H',bytearray(data[8:8+2]))[0] # Week number
    offset = offset + 2
    
    value, slip = read_double(data,offset, min=None, max=1E10, tries=5)
    offset = offset + 8 + slip
    gps_time_shift = value # GPS-UTC time shift [ms]

    value, slip = read_double(data,offset, min=None, max=1E10, tries=5)
    offset = offset + 8 + slip
    glo_time_shift = value # GLONASS-UTC time shift [ms]

    rec_t_corr = struct.unpack('<b',bytearray(data[offset:offset+1]))[0] # Receiver Time Scale Correction [ms]
    offset = offset +1

    # 30*number of channels used
    num_channels = int((len(data) - offset)/30)
    # Create storage structures
    signal_type = [] # 1-GLONASS, 2-GPS, 4-SBAS
    sat_number = []
    carrier_num = [] # for glonass
    snr = [] # dB-Hz
    carrier_phase = [] # cycles
    pseudo_range = [] # ms
    doppler_freq = [] # Hz
    flags = []


    # Process data
    for i in range(0,num_channels):  
        signal_type.append(struct.unpack('<B',bytearray(data[offset:offset+1]))[0])
        offset = offset + 1
        sat_number.append(struct.unpack('<B',bytearray(data[offset:offset+1]))[0])
        offset = offset + 1
        carrier_num.append(struct.unpack('<b',bytearray(data[offset:offset+1]))[0])
        offset = offset + 1
        snr.append(struct.unpack('<B',bytearray(data[offset:offset+1]))[0])
        offset = offset + 1

        value, slip = read_double(data,offset, min=1e-50, max=1E10, tries=5)
        offset = offset + 8 + slip
        carrier_phase.append(value)

        value, slip = read_double(data,offset, min=1e-50, max=1E10, tries=5)
        offset = offset + 8 + slip
        pseudo_range.append(value)

        value, slip = read_double(data,offset, min=1e-50, max=1E5, tries=5)
        offset = offset + 8 + slip
        doppler_freq.append(value) 

        flags.append(struct.unpack('<B',bytearray(data[offset:offset+1]))[0])
        offset = offset + 1
        offset = offset + 1
         
    return {"Time":tm, "Week Number": week_num, "GPS time shift": gps_time_shift, 
            "GLO time shift": glo_time_shift, "Rec Time Scale Correction": rec_t_corr,
            "Signal Type":signal_type, "Sat Number":sat_number,
            "Carrier Number":carrier_num, "SNR":snr,
            "Carrier Phase":carrier_phase, "Pseudo Range": pseudo_range,
            "Doppler Freq":doppler_freq, "Flags":flags}

def read_double(data, offset, min=None, max=1E10, allow_negative=True, tries=1):
    """
    Reads a double from the given byte stream at a specified offset position.
    The binr raw_data sometimes has a 0 in front of float values. Not sure where 
    they come from. At the moment this is easily handled by detecting an out of
    range value and start reading the double 1 byte later.

    arguments: 
        data - list of bytes to read double from
        offset - index to start reading double
        min - minimum valid value
        max - maximum valid value
        tries - number of tries allowed (to avoid infinite loops)
        allow_negative - if False, range error will be triggered on negative values

    returns:
        value - double value that has been read
        slip - number of spurious zeros we skipped
    """
    slip = 0 # keeps track of the number of zero jumps
    tried = 0 # keeps track of the number of times we tried
    
    range_error = True # Error cleared when a valid value is read.
    while tried < tries and range_error:
        # Try and read the value
        value = struct.unpack('<d',bytearray(data[offset+slip:offset+slip+8]))[0]

        # Check if the value is stupid
        if (np.abs(value) > max or 
           (not (min == None) and np.abs(value) < min) and (not value == 0) or 
           (not allow_negative and value < 0)):
            slip = slip + 1
        else:
            range_error = False # It was valid
        
        tried = tried + 1
    
    return value, slip
        
            
        

def print_raw_data(raw_data):
    """
    Print the processed result of the status of receiver channel request.
    """
    print("Time: "+str(raw_data["Time"]))
    print("Week: "+str(raw_data["Week Number"]))
    print("GPS time shift: "+str(raw_data["GPS time shift"]))
    print("GLO time shift: "+str(raw_data["GLO time shift"]))
    print("Rec Time Scale Correction: "+str(raw_data["Rec Time Scale Correction"]))
    num_channels = len(raw_data["Signal Type"])
    print("Number of channels: "+str(num_channels))
    print("---------------------------------------------------")
    print("Ch\tSystem(ID)\tCarrier\tSNR\tPhase\t\tPseudorange\tDoppler\t\tSignal\tPrngDop\tSmooth\tCarrier\tFPrng\tNo Pre-amble")
    for i in range(20):
        if i < num_channels:
            if raw_data["Signal Type"][i] == 1:
                system_name = "GLO"+"("+str(raw_data["Sat Number"][i])+")"
            elif raw_data["Signal Type"][i] == 2:
                system_name = "GPS"+"("+str(raw_data["Sat Number"][i])+")"
            elif raw_data["Signal Type"][i] == 4:
                system_name = "SB"+"("+str(raw_data["Sat Number"][i])+")"
            else:
                system_name = raw_data["Signal Type"][i]
            fmt = "{:.5E}"
            print(str(i)+"\t"+str(system_name)+"\t\t"+str(raw_data["Carrier Number"][i])+"\t"+
                str(raw_data["SNR"][i])+"\t"+fmt.format(raw_data["Carrier Phase"][i])+"\t"+fmt.format(raw_data["Pseudo Range"][i])+"\t"+
                fmt.format(raw_data["Doppler Freq"][i])+"\t"+
                str(raw_data["Flags"][i]&0x01>0)+"\t"+
                str(raw_data["Flags"][i]&0x02>0)+"\t"+
                str(raw_data["Flags"][i]&0x04>0)+"\t"+
                str(raw_data["Flags"][i]&0x08>0)+"\t"+
                str(raw_data["Flags"][i]&0x10>0)+"\t"+
                str(raw_data["Flags"][i]&0x20>0)+"\t")
        else:
            print("---")

def process_geocentric_coordinates_of_antenna(data):
    """
    Process the geocentric coordinate packet. Sent every minute 
    when request for raw data output has been activated.

    arguments:
        data - packet F6h data

    return:
        {X, Y, Z, X error, Y error, Z error, Flag}
    """

    # Extract data from packet
    X = struct.unpack('<d',bytearray(data[0:0+8]))[0] # [m]
    Y = struct.unpack('<d',bytearray(data[8:8+8]))[0] # [m]
    Z = struct.unpack('<d',bytearray(data[16:16+8]))[0] # [m]
    Xerror = struct.unpack('<d',bytearray(data[24:24+8]))[0] # [m] rms error
    Yerror = struct.unpack('<d',bytearray(data[32:32+8]))[0] # [m] rms error
    Zerror = struct.unpack('<d',bytearray(data[40:40+8]))[0] # [m] rms error
    flag = struct.unpack('<B',bytearray(data[48:48+1]))[0] # Flag of user dynamic 0 - static, 1 - kinematic

    return {"X":X, "Y":Y, "Z":Z, 
            "X error":Xerror, "Y error":Yerror, "Z error":Zerror, 
            "Flag": flag}


def process_software_version(data):
    """
    Process the software version packet. Sent a single time after a
    raw data request.

    arguments:
        data - packet 1Bh data
    
    return:
        {"No of Channels", "Version", "Serial Number"}
    """
    no_of_channels = struct.unpack('<B',bytearray(data[0:0+1]))[0]
    #TODO: resolve this correctly 21 byte long string
    version = struct.unpack('<B',bytearray(data[1:1+1]))[0]
    serial = struct.unpack('<I',bytearray(data[22:22+4]))[0]

    return {"No of Channels":no_of_channels, "Version": version,
            "Serial Number": serial}


def process_ionosphere_parameters(data):
    """
    Process the ionosphere parameters from packet 2Ah. Sent every 
    2 minutes after a raw data request.

    arguments:
        data - packet 4Ah

    return:
        {"alpha_0", "alpha_1", "alpha_2", "alpha_3",
         "beta_0", "beta_1", "beta_2", "beta_3", "Reliability"}
    """

    a0 = struct.unpack('<f',bytearray(data[0:0+4]))[0] # sec
    a1 = struct.unpack('<f',bytearray(data[4:4+4]))[0] # sec/semicycle
    a2 = struct.unpack('<f',bytearray(data[8:8+4]))[0] # sec/(semicycle)^2
    a3 = struct.unpack('<f',bytearray(data[12:12+4]))[0] # sec/(semicycle)^3

    b0 = struct.unpack('<f',bytearray(data[16:16+4]))[0] # sec
    b1 = struct.unpack('<f',bytearray(data[20:20+4]))[0] # sec/semicycle
    b2 = struct.unpack('<f',bytearray(data[24:24+4]))[0] # sec/(semicycle)^2
    b3 = struct.unpack('<f',bytearray(data[28:28+4]))[0]  # sec/(semicycle)^3

    reliability = struct.unpack('<B',bytearray(data[32:32+1]))[0] # 255 - data is reliable

    return {"alpha_0":a0, "alpha_1":a1, "alpha_2":a2, "alpha_3":a3,
            "beta_0":b0, "beta_1":b1, "beta_2":b2, "beta_3":b3, 
            "Reliability":reliability}

def process_time_scales_parameters(data):
    
    """
    Process the data received in packet 4Bh relating to 
    GPS, GLONASS and UTC time scales parameters. Sent every
    2 minutes after a data request.

    argurments:
        data - data from 4Bh packet

    return:
        {"A_1", "A_0", "t_ot","WN_t", "dt_LS", "WN_LSF", "DN", "dt_LSF",
         "GPS Reliability", "N^A", "tau_c", "GLONASS Reliability"}
    """

    A1 = struct.unpack('<d',bytearray(data[0:0+8]))[0]  # Sec/sec
    A0 = struct.unpack('<d',bytearray(data[8:8+8]))[0]  # Sec
    t_ot = struct.unpack('<f',bytearray(data[16:16+4]))[0]  # Sec
    WN_t = struct.unpack('<H',bytearray(data[20:20+2]))[0]  # Weeks
    dt_LS = struct.unpack('<h',bytearray(data[22:22+2]))[0]  # Sec
    WN_LSF = struct.unpack('<H',bytearray(data[24:24+2]))[0]  # Weeks
    DN = struct.unpack('<H',bytearray(data[26:26+2]))[0]  # Days
    dt_LSF = struct.unpack('<h',bytearray(data[28:28+2]))[0]  # Sec
    gps_rel = struct.unpack('<B',bytearray(data[30:30+1]))[0] # 255 - data is reliable
    # Number of the day to which the tau_c time stamp refers
    NA = struct.unpack('<H',bytearray(data[31:31+2]))[0]  
    tau_c = struct.unpack('<d',bytearray(data[33:33+8]))[0]  # Sec
    glo_rel = struct.unpack('<B',bytearray(data[41:41+1]))[0] # 255 - data is reliable
    
    return  {"A_1":A1, "A_0":A0, "t_ot":t_ot,"WN_t":WN_t, "dt_LS":dt_LS, 
             "WN_LSF":WN_LSF, "DN":DN, "dt_LSF":dt_LSF,
             "GPS Reliability":gps_rel, "N^A":NA, "tau_c":tau_c, 
             "GLONASS Reliability":glo_rel}

def process_extended_ephemeris_of_satellites(data):
    """
    Process the extended ephemeris packet F7h sent at the same interval
    as the raw data.

    arguments:
        data - data from F4h packet

    return:
        dictionary containing data listed on page 71 of the V1.3 BINR 
        protocol specification.
    """

    # Get the system
    sat_system = struct.unpack('<B',bytearray(data[0:0+1]))[0] # 1 - GPS, 2 - GLONASS
    sat_no = struct.unpack('<B',bytearray(data[1:1+1]))[0] 

    if sat_system == GPS: # Process the next 136 bytes
        c_rs = struct.unpack('<f',bytearray(data[2:2+4]))[0]  # m
        dn = struct.unpack('<f',bytearray(data[6:6+4]))[0]  # rad/ms
        m_0 = struct.unpack('<d',bytearray(data[10:10+8]))[0]  # rad
        c_uc = struct.unpack('<f',bytearray(data[18:18+4]))[0]  # rad
        e = struct.unpack('<d',bytearray(data[22:22+8]))[0]  # E
        c_us = struct.unpack('<f',bytearray(data[30:30+4]))[0]  # rad
        sqrtA = struct.unpack('<d',bytearray(data[34:34+8]))[0]  # m^1/2
        t_0e = struct.unpack('<d',bytearray(data[42:42+8]))[0]  # ms
        c_ic = struct.unpack('<f',bytearray(data[50:50+4]))[0]  # rad
        omega_0 = struct.unpack('<d',bytearray(data[54:54+8]))[0]  # rad
        c_is = struct.unpack('<f',bytearray(data[62:62+4]))[0]  # rad
        i_0 = struct.unpack('<d',bytearray(data[66:66+8]))[0]  # rad
        c_rc = struct.unpack('<f',bytearray(data[74:74+4]))[0]  # rad
        w = struct.unpack('<d',bytearray(data[78:78+8]))[0]  # rad
        omega_dot = struct.unpack('<d',bytearray(data[86:86+8]))[0]  # ms
        idot = struct.unpack('<d',bytearray(data[94:94+8]))[0]  # rad/ms
        t_gd = struct.unpack('<f',bytearray(data[102:102+4]))[0]  # ms
        t_0c = struct.unpack('<d',bytearray(data[106:106+8]))[0]  # ms
        a_f2 = struct.unpack('<f',bytearray(data[114:114+4]))[0]  # ms/ms^2
        a_f1 = struct.unpack('<f',bytearray(data[118:118+4]))[0]  # ms/ms
        a_f0 = struct.unpack('<f',bytearray(data[122:122+4]))[0]  # ms
        ura = struct.unpack('<H',bytearray(data[126:126+2]))[0]  
        iode = struct.unpack('<H',bytearray(data[128:128+2]))[0]  
        iodc = struct.unpack('<H',bytearray(data[130:130+2]))[0]
        codel2 = struct.unpack('<H',bytearray(data[132:132+2]))[0]  
        l2_pdata_flag = struct.unpack('<H',bytearray(data[134:134+2]))[0]  
        weekN = struct.unpack('<H',bytearray(data[136:136+2]))[0]

        return {"System":sat_system, "PRN":sat_no, "C_rs":c_rs, "dn":dn, "M_0":m_0,
                "C_uc":c_uc, "e":e,"C_us":c_us,"sqrtA":sqrtA, "t_0e":t_0e,
                "C_ic":c_ic, "Omega_0":omega_0, "C_is":c_is, "I_0":i_0, 
                "C_rc":c_rc, "w":w, "Omega_dot":omega_dot, "IDOT":idot,
                "T_GD":t_gd, "t_0c":t_0c, "a_f2":a_f2, "a_f1":a_f1, 
                "a_f0":a_f0, "URA":ura, "IODE":iode, "IODC":iodc, 
                "CODEL2":codel2, "L2 P Data Flag":l2_pdata_flag, "WN":weekN}

    elif sat_system == GLONASS: # 92 bytes
        HnA = struct.unpack('<b',bytearray(data[2:2+1]))[0] # Carrier frequency number
        xn = struct.unpack('<d',bytearray(data[3:3+8]))[0] # Coordinates [m]
        yn = struct.unpack('<d',bytearray(data[11:11+8]))[0] # Coordinates [m]
        zn = struct.unpack('<d',bytearray(data[19:19+8]))[0] # Coordinates [m]
        xnv = struct.unpack('<d',bytearray(data[27:27+8]))[0] # Speed [m/ms]
        ynv = struct.unpack('<d',bytearray(data[35:35+8]))[0] # Speed [m/ms]
        znv = struct.unpack('<d',bytearray(data[43:43+8]))[0] # Speed [m/ms]
        xna = struct.unpack('<d',bytearray(data[51:51+8]))[0] # acceleration [m/ms^2]
        yna = struct.unpack('<d',bytearray(data[59:59+8]))[0] # acceleration [m/ms^2]
        zna = struct.unpack('<d',bytearray(data[67:67+8]))[0] # acceleration [m/ms^2]
        tb = struct.unpack('<d',bytearray(data[75:75+8]))[0] # Time interval inside the current day [msec]
        gamma_n = struct.unpack('<f',bytearray(data[83:83+4]))[0] # Signal-carrier frequency value relative deviation
        tau_n = struct.unpack('<f',bytearray(data[87:87+4]))[0] # Satellite time scale offset value in relation to the GLONASS scale [ms]
        e_n = struct.unpack('<H',bytearray(data[91:91+2]))[0] # Satellite time scale offset value in relation to the GLONASS scale [ms]

        return {"System":sat_system, "PRN":sat_no, "H_n^A":HnA,
                "x_n":xn, "y_n":yn, "z_n":zn, 
                "x_nv":xnv, "y_nv":ynv, "z_nv":znv,
                "x_na":xna, "y_na":yna, "z_na":zna,
                "t_b":tb, "gamma_n":gamma_n, "tau_n":tau_n,
                "E_n":e_n}

    else:
        raise ValueError("Invalid system byte")    
        
        
        
        

        
        

 
