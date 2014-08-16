#include "DifferentialPCFanMotors.h"
#include "pinConfig.h"

//For communicating with GPS. This should be changed to hardware UART after development and debugging.
#include <SoftwareSerial.h>

/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24Network.h>
#include <RF24.h>

#include <Wire.h> //I2C Arduino Library

/*Setup software UART port*/
#define GPSRATE 9600
#define BUFFSIZ 90 // plenty big

char buffidx;
char buffer[BUFFSIZ];

SoftwareSerial mySerial =  SoftwareSerial(GPS_TX, GPS_RX);

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
#define REPLY_HEARTBEAT 4
#define RECEIVE_ERROR 200

/*Assigns memory for the payload*/
byte payload[8]; 

/*System variables*/
byte state = 2;
byte buoyCode = 0;

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
  /*Update the mesh network*/
  network.update();
  checkIfPacketsAreAvailable();  
  
  /*Read sensors*/
  
  /*Navigation*/
  
  
  /*Test peripherals*/
  //char buffer[10];
  //itoa(radio.getPayloadSize (),buffer,10);
  //Serial.println(buffer);
  

  //readGPSRaw();
}

void checkIfPacketsAreAvailable()
{
  /*Handle radio*/
  if(network.available())
  {
    Serial.println("Received request");

     //Fetch the data payload
     RF24NetworkHeader header;
     network.read(header,&payload,sizeof(payload)); 
    
    delay(20); 
    if (payload[0] == REQUEST_HEARTBEAT)
    {
      
      /*Build heartbeat payload*/
      payload[0] = REPLY_HEARTBEAT;
      payload[1] = buoyCode;
      payload[2] = state;
      
      /*Send off msg*/
      Serial.println("Sending heartbeat Packet");
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
    //radio.setRetries(15,15);
    network.begin(/*channel*/ 90, /*node address*/ BUOY_G1_ADDRESS);
    //radio.setPayloadSize(9);
    /*radio.openWritingPipe(controlRXPipe);
    radio.openReadingPipe(1,controlTXPipe);
    radio.startListening();*/
}
