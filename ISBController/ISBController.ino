/*
Code for ISB Controller. The ISB Controller is a user interface 
 for the ISB project. This firmware controls an LCD and rotary 
 encoder to give the user control through menus. Data can then 
 be sent between the ISB system and the controller through a 
 tranceiver 
 
 Thanks to Adafruit for their LCD Library and Tutorials.
 Thanks to 0xPIT for his rotary encoder library
 Thanks to the Arduino libraries used in this project
 */


/*Tranceiver includes*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*General Includes*/
#include "pins.h"
#include "configuration.h"
#include "gui.h"

/*Include LCD*/
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
/*Rotary Encoder Includes*/
#include <ClickEncoder.h>
#include <TimerOne.h>

/*GUI global variables*/
/*LCD initialisation on software SPI, hardware SPI will be used for the tranceiver*/
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_CLK,LCD_DIN,LCD_DC, LCD_CS, LCD_RST);

/*Encoder initialisation using defined pins and steps-per-notch*/
ClickEncoder *encoder = new ClickEncoder(ENCODER_DT, ENCODER_CLK, ENCODER_SWITCH,ENCODER_STEPS_PER_NOTCH);


/*RF module variables*/
const uint64_t controlRXPipe = 0xE8E8F0F0E1LL; // Define the transmit pipe
const uint64_t controlTXPipe = 0xAAAAF0A0BBLL; // Define the transmit pipe
RF24 radio(RF_CE, RF_CSN);

short heartbeatReqCount = 0;

void setup()   {
  Serial.begin(9600);

  /*Initialise rotary encoder used to control the interface
   and screen used to display the interface*/
  initEncoder();
  initScreen(MENU_STATUS,MENU_BACK); 
  initRF();

  /*Show intro*/
  splashScreen();
  delay(2000);

}


void loop() {
  byte msg[2];
  
  
  /*Ask buoys if they are alive*/
  if(heartbeatReqCount == 0)
  {
     //Send heartbeat request (0)
     msg[0] = 0;
     
     radio.stopListening();
     
     bool sendStatus = radio.write(&msg, sizeof(msg));
     
     //Check if msg could be sent succesfully
     if (sendStatus)
       Serial.println("Heartbeat Request Sent...");
     else
       Serial.println("Failed");
     
     //Switch back to listening mode  
     radio.startListening();
  }
  
  /*Test peripherals*/
  if (radio.available() )
  {
    Serial.println("I received something");
    // Read the data payload until we've received everything
    bool done = false;
    while (!done)
    {
       //Fetch the data payload
        done = radio.read(msg, sizeof(msg) );
        //  Serial.print(msg);
    }
    

    
  }
  


  /*Start with main routine*/
  //update all systems

  /*Check controls and handle user input*/
  controlUpdate();

  /*Update LCD*/
  screenUpdate();


  heartbeatReqCount += 1;
}



/*Initialise RF module to state where it is connected to the course*/
void initRF()
{    
    radio.begin();
    
    radio.setRetries(15,15);
    radio.setPayloadSize(8);
    
    radio.openWritingPipe(controlTXPipe);
    radio.openReadingPipe(1,controlRXPipe);
    radio.startListening();
    //radio.openWritingPipe(pipe);
    
}








