#include "DifferentialPCFanMotors.h"
#include "pinConfig.h"
#include "communication.h"

/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24Network.h>
#include <RF24.h>
/*GPS includes*/
#include <TinyGPS.h>
#include <SoftwareSerial.h>
/*Compass includes*/
#include <Wire.h> 

/*Setup software UART port*/
#define GPSRATE 9600
TinyGPS gps;
SoftwareSerial mySerial =  SoftwareSerial(GPS_TX, GPS_RX);
#define RX_BUFFER_SIZE 64
static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_head = 0;
static volatile uint8_t rx_buffer_tail = 0;


#define address 0x1E //0011110b, I2C 7bit address of HMC5883

/*RF module variables*/
RF24 radio(RF_CE, RF_CSN);
//Network uses that radio
RF24Network network(radio);

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

/*GPS storage variables*/
long gpsLat = 1000;
long gpsLon = 2000;
int battery = 0;
bool gpsFix = false;
unsigned long fix_age;
int compassHeading = 0;

void setup() {  
  /*Initialise GPS*/
  initGPS();
  
  /*Initialise Radio*/
  initRF();
  
  /*Initialise Compass*/
  initCompass();
  
  //Initialise debugging Serial
  Serial.begin(9600);  
}


void loop() 
{
  /*Update the mesh network*/
  network.update();
  checkIfPacketsAreAvailable();  
  
  /*Read sensors*/
  readGPS();
  readCompass();
  readBattery();
  
  /*Navigation*/
  
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

/*Reads the current voltage of the battery directly from the cell*/
void readBattery()
{
   int batteryRaw = analogRead(BAT);
   battery = (int)((5000.0/1023.0)*(batteryRaw));
}

void readCompass()
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
  compassHeading = (heading * 180 / M_PI);
}




/*Initialise GPS with all of its settings, from adafruit example*/
void initGPS()
{
  mySerial.begin(9600);
}

/*Updates GPS and its variables*/
void readGPS()
{
  while (mySerial.available())
  {
    int c = mySerial.read();
    if (gps.encode(c))
    {
      // process new gps info here
    }
  }
  
  // retrieves +/- lat/long in 100000ths of a degree
  gps.get_position(&gpsLat, &gpsLon, &fix_age);   
}  

/*Sets up I2C communication to compass module*/
void initCompass()
{
  Wire.begin();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();
}



