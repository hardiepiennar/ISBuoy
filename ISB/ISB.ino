#include "DifferentialPCFanMotors.h"
#include "pinConfig.h"

//For communicating with GPS. This should be changed to hardware UART after development and debugging.
#include <SoftwareSerial.h>

/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h> //I2C Arduino Library

/*Setup software UART port*/
#define GPSRATE 9600
#define BUFFSIZ 90 // plenty big

char buffidx;
char buffer[BUFFSIZ];

SoftwareSerial mySerial =  SoftwareSerial(GPS_TX, GPS_RX);

/*RF module variables*/
const uint64_t controlRXPipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
const uint64_t controlTXPipe = 0xAAAAF0A0BBLL; // Define the transmit pipe
RF24 radio(RF_CE, RF_CSN);

#define address 0x1E //0011110b, I2C 7bit address of HMC5883


void setup() {  
  /*Test GPS*/
  Serial.begin(GPSRATE);
  mySerial.begin(GPSRATE);
  
  initRF();
  
  //Initialize Serial and I2C communications
  Serial.begin(9600);
  Wire.begin();
  
  //Put the HMC5883 IC into the correct operating mode
  Wire.beginTransmission(address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();
}

void loop() 
{
  /*Handle radio*/
  int msg[2];
  msg[1] = 5;
  msg[0] = 0;
  
  //Check if data has been sent on listening port
  if(radio.available())
  {
    Serial.println("I received a heartbeat");
    // Read the data payload until we've received everything
    bool done = false;
    while (!done)
    {
        
       //Fetch the data payload
        done = radio.read(msg, sizeof(msg) );
        
    }
    
    radio.stopListening();
     
    delay(20); 
    bool sendStatus = radio.write(&msg, sizeof(msg));
   
    //Switch back to listening mode  
    radio.startListening();
    
    
  }
  
  /*Read sensors*/
  
  /*Navigation*/
  
  
  /*Test peripherals*/
  //char buffer[10];
  //itoa(radio.getPayloadSize (),buffer,10);
  //Serial.println(buffer);
  

  
  delay(250);
  //readGPSRaw();
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

void readCompass()
{
  int x,y,z; //triple axis data

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
  
  //Print out values of each axis
  Serial.print("x: ");
  Serial.print(x);
  Serial.print("  y: ");
  Serial.print(y);
  Serial.print("  z: ");
  Serial.println(z);  
}

/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    radio.setRetries(15,15);
    radio.setPayloadSize(8);
    radio.openWritingPipe(controlRXPipe);
    radio.openReadingPipe(1,controlTXPipe);
    radio.startListening();
}
