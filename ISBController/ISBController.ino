/*
Code for ISB Controller. The ISB Controller is a user interface 
for the ISB project. This firmware controls an LCD and rotary 
encoder to give the user control through menus. Data can then 
be sent between the ISB system and the controller through a 
tranceiver 

Thanks to Adafruit for their LCD Library and Tutorials.
Thanks to 0xPIT for his rotary encoder library
*/

/*LCD Includes*/
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

/*Rotary Encoder Includes*/
#include <ClickEncoder.h>
#include <TimerOne.h>

/*General Includes*/
#include "pins.h"
#include "configuration.h"

/*GUI global variables*/
/*LCD initialisation on hardware SPI (SPI cannot be used for any other peripherals)*/ 
Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_DC, LCD_CS, LCD_RST);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

/*Encoder initialisation using defined pins and steps-per-notch*/
ClickEncoder *encoder = new ClickEncoder(ENCODER_DT, ENCODER_CLK, ENCODER_SWITCH,ENCODER_STEPS_PER_NOTCH);

/*Keeps track of the current menu states*/
short menuState;
short innerMenuState;
short noOfMenuItems;

/*Settings*/
int contrast = 57;

void setup()   {
  Serial.begin(9600);
  
  /*Initialise rotary encoder used to control the interface
  and screen used to display the interface*/
  initEncoder();
  initScreen(); 
  
  /*Show intro*/
  splashScreen();
  delay(2000);
  
  /*Initialise menu system*/
  menuState = MENU_STATUS; //Set the menu to status
  innerMenuState = -1;
  noOfMenuItems = 0;
  
}


void loop() {
  /*Start with main routine*/
  //update all systems
  
  /*Check controls and handle user input*/
  controlUpdate();
  
  /*Update LCD*/
  screenUpdate();
  
  delay(100);//Temporary for testing
}

/*Checks rotary encoder status and reacts to input accordingly*/
void controlUpdate()
{
  /*Handle the turning of the encoder by checking the direction
  turned by the encoder*/  
  /*update encoder value*/
  int encoderValue = encoder->getValue();
    
  /*Check if encoder state changed*/
  if (encoderValue != 0) 
  {
    /*Encoder button was pressed, handle event according to 
    current menu status*/
    switch(menuState)
    {
      case MENU_STATUS:
        
        break;
      
      case MENU_MAIN:
      case MENU_SETTINGS:
        innerMenuState += encoderValue;
        /*Keep cursor from selecting invalid option*/
        if(innerMenuState < -1)
          innerMenuState = MENU_BACK;
        if(innerMenuState >= noOfMenuItems)
          innerMenuState = noOfMenuItems - 1;
      break;
      
      /*Change contrast value and set the new display contrast*/
      case MENU_SETTINGS_CONTRAST:
        contrast += encoderValue;
        display.setContrast(contrast);
        break;
    } 
  }
  
  /*Handle encoder button events. Various states can be checked,
  however only the released state is really neccesary*/
  if(encoder->getButton() == ClickEncoder::Clicked)
  {
    /*Encoder button was pressed, handle event according to 
    current menu status*/
    switch(menuState)
    {
      case MENU_STATUS:
        /*Change into main menu*/
        menuState = MENU_MAIN;
        /*Reset menu specific variables*/
        innerMenuState = MENU_BACK; 
        noOfMenuItems = 0;
        break;
      
      case MENU_MAIN:
        /*Check which menu option is selected*/
        switch(innerMenuState)
        {
           case MENU_BACK:
             menuState = MENU_STATUS;
             innerMenuState = MENU_BACK;
             noOfMenuItems = 0;
             break; 
             
           case MENU_SETTINGS_PLACEMENT:
             menuState = MENU_SETTINGS;
             innerMenuState = MENU_BACK;
             noOfMenuItems = 0;
             break; 
        }
      break;
      
      case MENU_SETTINGS:
        /*Check which menu option is selected*/
        switch(innerMenuState)
        {
           case MENU_BACK:
             menuState = MENU_MAIN;
             innerMenuState = MENU_BACK;
             noOfMenuItems = 0;
             break; 
           
           case MENU_CONTRAST_PLACEMENT:
             menuState = MENU_SETTINGS_CONTRAST;
             innerMenuState = MENU_BACK;
             noOfMenuItems = 0;
             break; 
        }
      break;
      
      case MENU_SETTINGS_CONTRAST:
        menuState = MENU_SETTINGS;
        innerMenuState = MENU_CONTRAST_PLACEMENT;
        noOfMenuItems = 0;
        break;
      
    } 
  }
}

/*Draws the screen according to the current menu 
and system status*/
void screenUpdate()
{
  /*Clear Screen*/
  display.clearDisplay();
  
  switch(menuState)
  {
    case MENU_STATUS:
      /*Draw header*/
      drawHeader("STATUS");
      break;
    
    case MENU_MAIN:
      /*Draw header*/
      drawHeader("MENU");
      /*Draw menu items*/
      noOfMenuItems = 0;
      drawMenuItem("SESSION",MENU_SESSION_PLACEMENT);
      drawMenuItem("COURSE",MENU_COURSE_PLACEMENT);
      drawMenuItem("BUOYS",MENU_BOUYS_PLACEMENT);      
      drawMenuItem("SETTINGS",MENU_SETTINGS_PLACEMENT);
      /*Draw selected cursor*/
      drawCursor();
    break;

    case MENU_SETTINGS:
    case MENU_SETTINGS_CONTRAST:
      /*Draw header*/
      drawHeader("SETTINGS");
      /*Draw menu items*/
      noOfMenuItems = 0;
      if(menuState == MENU_SETTINGS_CONTRAST)
        drawMenuSliderItem("CONTRAST",MENU_CONTRAST_PLACEMENT,contrast,0,100,1);
      else
        drawMenuSliderItem("CONTRAST",MENU_CONTRAST_PLACEMENT,contrast,0,100,0);
      /*Draw selected cursor*/
      drawCursor();
      break;
      
     
  } 
  
  /*Redraw Screen*/
  display.display();
}

/*Draws cursor at selected menu item using the globally defined
selected item variable*/
void drawCursor()
{
  display.setTextColor(BLACK);
  
  /*if innerMenuState is -1, the cursor is drawn over the back
  menu item*/  
  int index = innerMenuState;
  if(index == -1)
  {      
    display.fillRect(BACK_BUTTON_START,0,BACK_BUTTON_CURSOR_WIDTH,LINE_HEIGHT,BLACK);
  }
  else
  {
     display.fillRect(0,index*(LINE_HEIGHT +LINE_SPACER) + HEADER_HEIGHT+LINE_SPACER,CURSOR_WIDTH,LINE_HEIGHT,BLACK);
  }
}

/*Draws a menu item with a slider function, the menu item displays the current value when inactive.
When active, the string is replaced by the slider which is then drawn as specified with min max and
its current value*/
void drawMenuSliderItem(char* string,short index,int value,int minValue,int maxValue,short active)
{
    int barLength = 60;
    char buffer[3];
    display.setTextColor(BLACK);
    display.setTextSize(1);
    
    if(active == 0)
    {
      //Draw name of slider
      display.setCursor(CURSOR_WIDTH+1,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
      display.println(string);           
    }
    else
    {
      display.drawRect(0,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER,barLength,LINE_HEIGHT,BLACK);
      display.fillRect(0,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER,(int)((barLength*value/(maxValue-minValue))),LINE_HEIGHT,BLACK);
    }
    
    //Draw slider value
    display.setCursor(display.width() - 20,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
    display.println(itoa(value,buffer,10));
    
    /*Increase the number of menu items available*/
    noOfMenuItems += 1;
}
/*Draws a menu item under the header location with a 
given index. This allows menu items to be easily swapped
in the configuration. Menu items should have names 10 
characters or less, its assumed that the 0 state is the
root state, therefore it will not have any back
button*/
void drawMenuItem(char* string,short index)
{
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(CURSOR_WIDTH+1,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
    display.println(string);
    
    /*Increase the number of menu items available*/
    noOfMenuItems += 1;
}

/*Create a header at the top of the screen*/
void drawHeader(char* string)
{
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println(string);
    display.drawLine(0,(HEADER_HEIGHT- LINE_SPACER),display.width(),(HEADER_HEIGHT)- LINE_SPACER,BLACK);
    display.drawLine(0,(HEADER_HEIGHT- LINE_SPACER)-1,display.width(),(HEADER_HEIGHT)- LINE_SPACER-1,BLACK);
    
    if(menuState != MENU_STATUS)
    {
      display.setCursor(BACK_BUTTON_START + BACK_BUTTON_CURSOR_WIDTH + 1,0);
      display.println("Back");
    }
}

/*Shows the Intelligent Ski-Course splash screen*/
void splashScreen()
{
    display.clearDisplay();   
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(3,2);
    display.println(" INTELLIGENT");
    display.setTextSize(2);
    display.setCursor(0,14);
    display.println("  SKI");
    display.setCursor(6,32);
    display.println("COURSE");  
    display.display();    
}

/*Initialise encoder by setting up interrupts*/
void initEncoder()
{  
  /*Initialise interrupt to check button status*/
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
}

/*Initialise screen through with SPI bus with defined pins*/
void initScreen()
{  
  /*Initialise screen*/
  display.begin();
  
  /*Set the contrast of the display*/
  display.setContrast(contrast);
}

/*Interrupt on encoder events*/
void timerIsr() {
  encoder->service();
}
