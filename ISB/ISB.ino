#include "DifferentialPCFanMotors.h"
#include "pinConfig.h"

//For communicating with GPS. This should be changed to hardware UART after development and debugging.
#include <SoftwareSerial.h>

/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24Network.h>
#include <RF24.h>
#include <Adafruit_GPS.h>
#include <Adafruit_GPS.h>
#include <Wire.h> //I2C Arduino Library

/*Setup software UART port*/
#define GPSRATE 9600
#define BUFFSIZ 90 // plenty big
char buffidx;
char buffer[BUFFSIZ];
SoftwareSerial mySerial =  SoftwareSerial(GPS_TX, GPS_RX);
Adafruit_GPS GPS(&mySerial);

//GPS Settings
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  false
// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy
uint32_t timer = millis();


#define address 0x1E //0011110b, I2C 7bit address of HMC5883

/*RF module variables*/
RF24 radio(RF_CE, RF_CSN);
//Network uses that radio
RF24Network network(radio);

/*Define radio addresses*/
#define CONTROLLER_ADDRESS 0
#define BUOY_G1_ADDRESS 1

/*Define message types*/
#define REQUEST_HEARTBEAT 0
#define REQUEST_BAT 1
#define REQUEST_GPS 2
#define REQUEST_MAG 3
#define REQUEST_GPS_FIX 4
#define REQUEST_GPS_LAT 5
#define REQUEST_GPS_LON 6
#define REPLY_HEARTBEAT 7
#define REPLY_MAG 8
#define REPLY_GPS_FIX 9
#define REPLY_GPS_LAT 10
#define REPLY_GPS_LON 11
#define RECEIVE_ERROR 200


/*Assigns memory for the payload*/
byte payload[8]; 

/*System variables*/
byte state = 2;
byte buoyCode = 0;

/*Compass calibration storage
TODO: add a function to calibrate 
and store new values in eeprom*/
int xMax = -149;
int xMin = -569;
int yMax = 276;
int yMin = -144;
int declinationAngle  = 0;
/*Compass storage variables*/


void setup() {  
  /*Initialise GPS*/
  initGPS();
  
  /*Initialise Radio*/
  initRF();
  
  //Initialise Serial and I2C communications
  Serial.begin(9600);
  Wire.begin();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();
}


void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void loop() 
{
  /*Update the mesh network*/
  network.update();
  checkIfPacketsAreAvailable();  
  
  /*Read sensors*/
  readGPS();
  
  /*Navigation*/

  
  /*Test peripherals*/
  //char buffer[10];
  //itoa(radio.getPayloadSize (),buffer,10);
  //Serial.println(buffer);
  //readCompass();

  //readGPSRaw();
}

void checkIfPacketsAreAvailable()
{
  /*Handle radio*/
  if(network.available())
  {
    //Serial.println("Received request");

     //Fetch the data payload
    RF24NetworkHeader header;
    network.read(header,&payload,sizeof(payload)); 
    delay(20);
    
    if (payload[0] == REQUEST_HEARTBEAT)
    {      
      Serial.println("R HB"); 
      /*Build heartbeat payload*/
      payload[0] = REPLY_HEARTBEAT;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Send off msg*/
      //Serial.println("Sending heartbeat Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));   
      delay(20);   
    }   
    else if (payload[0] == REQUEST_MAG)
    {      
      Serial.println("R M"); 
      /*Build heartbeat payload*/
      payload[0] = REPLY_MAG;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Copy int variable into array*/
      int compassHeading = readCompass();
      const void* buf = &compassHeading;      
      memcpy(&payload[3],buf,sizeof(int));

      /*Send off msg*/
      //Serial.println("Sending MAG Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));
 
    } 
    else if (payload[0] == REQUEST_GPS_FIX)
    {    
      Serial.println("R GF");  
      /*Build heartbeat payload*/
      payload[0] = REPLY_GPS_FIX;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Copy int variable into array*/
      byte noOfSats = 10;
      const void* buf = &noOfSats;      
      memcpy(&payload[3],buf,sizeof(byte));
      /*Send off msg*/
      //Serial.println("Sending MAG Packet");
      RF24NetworkHeader header(/*to node*/ CONTROLLER_ADDRESS);
      bool sendStatus = network.write(header,&payload,sizeof(payload));    
    }   
    
    
  }
}

void testMotors()
{
    /*Motor Wet Long-run Test*/ 
    setLeftThrust(255);
    delay(60000);
    setLeftThrust(125);
    delay(60000);
    setLeftThrust(0);
    setRightThrust(255);
    delay(60000);
    setRightThrust(125);
    delay(60000);
    setLeftThrust(255);
    setRightThrust(255);
    delay(60000);
    setLeftThrust(125);
    setRightThrust(125);
    delay(60000);
    setLeftThrust(0);
    setRightThrust(0);
}

void readGPSRaw()
{
    Serial.print("\n\rread: ");
    readline(); 
}

void readline(void) {
  char c;
  
  buffidx = 0; // start at begninning
  while (1) {
      c=mySerial.read();
      if (c == -1)
        continue;
      Serial.print(c);
      if (c == '\n')
        continue;
      if ((buffidx == BUFFSIZ-1) || (c == '\r')) {
        buffer[buffidx] = 0;
        return;
      }
      buffer[buffidx++]= c;
  }
}

int readCompass()
{
  int x,y,z; //triple axis data
  
  /*Scaled values for compass, between 0 and 1*/
  float compassX = 0;
  float compassY = 0;
  float heading = 0;

  //Tell the HMC5883L where to begin reading data
  Wire.beginTransmission(address);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
  
 
 //Read data from each axis, 2 registers per axis
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read(); //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read(); //Y lsb
  }
  
  /*Calculate scaled values*/
  compassX = (((float)x-xMin)/(xMax-xMin))*2 - 1;
  compassY = (((float)y-yMin)/(yMax-yMin))*2 - 1;
  
  heading = atan2(compassY, compassX)+declinationAngle;
  //Correct for when signs are reversed.
  if(heading < 0)
    heading += 2 * PI;
  //Check for wrap due to addition of declination.
  if(heading > 2 * PI)
    heading -= 2 * PI;
  //Convert radians to degrees for readability.
  heading = (heading * 180 / M_PI);
  
  /*Check max and min values*/
  /*if(x > xMax) xMax = x;
  if(x < xMin) xMin = x;
  if(y > yMax) yMax = y;
  if(y < yMin) yMin = y;*/
  
  //Print out values of each axis
  /*Serial.print("Scaled X: ");
  Serial.print(compassX);
  Serial.print("Scaled Y: ");
  Serial.print(compassY);
  Serial.print("  Heading  ");
  Serial.println(heading);
  Serial.print("x: ");
  Serial.print(x);
  Serial.print("  y: ");
  Serial.print(y);
  Serial.print("  z: ");
  Serial.print(z);
  Serial.print("  x max: ");
  Serial.print(xMax); 
  Serial.print("  y max: ");
  Serial.print(yMax);   
  Serial.print("  x min: ");
  Serial.print(xMin); 
  Serial.print("  y min: ");
  Serial.println(yMin); */
  
  return heading;
}


/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ BUOY_G1_ADDRESS);
}

/*Initialise GPS with all of its settings, from adafruit example*/
void initGPS()
{
  GPS.begin(9600); 
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}

void readGPS()
{
   // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
  if (! usingInterrupt) {
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
    if (GPSECHO)
      if (c) Serial.print(c);
  }
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }
}  

// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

