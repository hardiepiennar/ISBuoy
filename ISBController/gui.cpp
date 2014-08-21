#include "gui.h"
#include "pins.h"
#include "configuration.h"

/*Include LCD*/
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

/*Rotary Encoder Includes*/
#include <ClickEncoder.h>
#include <TimerOne.h>

/*GUI global variables*/
/*LCD initialisation on software SPI, hardware SPI will be used for the tranceiver*/
extern Adafruit_PCD8544 display;

/*Encoder initialisation using defined pins and steps-per-notch*/
extern ClickEncoder *encoder;

/*Keeps track of the current menu states*/
int menuState;
int innerMenuState;
int noOfMenuItems;
short screenChanged;

/*Settings variables*/
int contrast = 57;

/*Course variables*/
int courseWidth = 11;
int courseLength = 25;
int courseTurns = 6;
extern bool buoyStatus[COURSE_SIZE];

extern int buoyCompass;
extern byte buoyNoSat;
extern long buoyLat;
extern long buoyLon;

void updateGUI()
{
   screenChanged = 1; 
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
    /*A screen update is neccesary*/
    screenChanged = 1;
    
    /*Encoder button was pressed, handle event according to 
     current menu status*/
    switch(menuState)
    {
      case MENU_STATUS:
  
        break;
  
      case MENU_MAIN:
      case MENU_SETTINGS:
      case MENU_COURSE:
      case MENU_SESSION:
      case MENU_BUOYS:
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
  
        if(contrast > CONTRAST_MAX)
          contrast = CONTRAST_MAX;
        else if(contrast < CONTRAST_MIN)
          contrast = CONTRAST_MIN;
  
        display.setContrast(contrast);
        break;
  
      /*Change the width setting of the course*/
      case MENU_COURSE_WIDTH:
        courseWidth += encoderValue;
  
        if(courseWidth > COURSE_MAX_WIDTH)
          courseWidth = COURSE_MAX_WIDTH;
        else if(courseWidth < COURSE_MIN_WIDTH)
          courseWidth = COURSE_MIN_WIDTH;
  
        break;
      /*Change the length setting of the course*/
      case MENU_COURSE_LENGTH:
        courseLength += encoderValue;
  
        if(courseLength > COURSE_MAX_LENGTH)
          courseLength = COURSE_MAX_LENGTH;
        else if(courseLength < COURSE_MIN_LENGTH)
          courseLength = COURSE_MIN_LENGTH;
  
        break;
        
      /*Change the turn setting of the course*/  
      case MENU_COURSE_TURNS:
        courseTurns += encoderValue;
  
        if(courseTurns > COURSE_MAX_TURNS)
          courseTurns = COURSE_MAX_TURNS;
        else if(courseTurns < COURSE_MIN_TURNS)
          courseTurns = COURSE_MIN_TURNS;
  
        break;
    } 
  }

  /*Handle encoder button events. Various states can be checked,
   however only the released state is really neccesary*/
  if(encoder->getButton() == ClickEncoder::Clicked)
  {
    /*A screen update is neccesary*/
    screenChanged = 1;
        
    /*Encoder button was pressed, handle event according to 
     current menu status*/
    switch(menuState)
    {
      
      case MENU_BUOYS:
        switch(innerMenuState)
        {
            case MENU_BACK:
              menuState = MENU_MAIN;
              innerMenuState = MENU_BACK;
              noOfMenuItems = 0;
              break; 
            
           default:
             menuState = MENU_BUOY;
             innerMenuState = MENU_BACK;
             noOfMenuItems = 0;
             break; 
        }
        break;
        
      case MENU_BUOY:      
        switch(innerMenuState)
        {
          case MENU_BACK:
            menuState = MENU_BUOYS;
            innerMenuState = MENU_BACK;
            noOfMenuItems = 0;
            break; 
            
        }
      break;
      
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
  
        case MENU_SESSION_PLACEMENT:
          menuState = MENU_SESSION;
          innerMenuState = MENU_BACK;
          noOfMenuItems = 0;
          break; 
  
        case MENU_COURSE_PLACEMENT:
          menuState = MENU_COURSE;
          innerMenuState = MENU_BACK;
          noOfMenuItems = 0;
          break; 
          
        case MENU_BUOYS_PLACEMENT:
          menuState = MENU_BUOYS;
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
  
    case MENU_SESSION:
    {
      switch(innerMenuState)
      {
        case MENU_BACK:
          menuState = MENU_MAIN;
          innerMenuState = MENU_BACK;
          noOfMenuItems = 0;
          break; 
      }
    }
    
    case MENU_COURSE:
      /*Check which menu option is selected*/
      switch(innerMenuState)
      {
        case MENU_BACK:
          menuState = MENU_MAIN;
          innerMenuState = MENU_BACK;
          noOfMenuItems = 0;
          break; 
  
        case MENU_COURSE_WIDTH_PLACEMENT:
          menuState = MENU_COURSE_WIDTH;
          innerMenuState = MENU_COURSE_WIDTH_PLACEMENT;
          noOfMenuItems = 0;
          break; 
  
        case MENU_COURSE_LENGTH_PLACEMENT:
          menuState = MENU_COURSE_LENGTH;
          innerMenuState = MENU_COURSE_LENGTH_PLACEMENT;
          noOfMenuItems = 0;
          break; 
          
        case MENU_COURSE_TURNS_PLACEMENT:
          menuState = MENU_COURSE_TURNS;
          innerMenuState = MENU_COURSE_TURNS_PLACEMENT;
          noOfMenuItems = 0;
          break; 
        }
        break;
  
      case MENU_COURSE_WIDTH:
        menuState = MENU_COURSE;
        innerMenuState = MENU_COURSE_WIDTH_PLACEMENT;
        noOfMenuItems = 0;
        break;
  
      case MENU_COURSE_LENGTH:
        menuState = MENU_COURSE;
        innerMenuState = MENU_COURSE_LENGTH_PLACEMENT;
        noOfMenuItems = 0;
        break;
        
      case MENU_COURSE_TURNS:
        menuState = MENU_COURSE;
        innerMenuState = MENU_COURSE_TURNS_PLACEMENT;
        noOfMenuItems = 0;
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
            innerMenuState = MENU_CONTRAST_PLACEMENT;
            noOfMenuItems = 0;
            break; 
        }
        break;
  
      /*Handle when slider is disabled*/
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
  /*Update LCD if neccesary*/
  if(!screenChanged)
    return;
  screenChanged = 0;
  
  /*Clear Screen*/
  display.clearDisplay();

  switch(menuState)
  {
  case MENU_STATUS:
    /*Draw header*/
    drawHeader("STATUS");
    
    /*Show Course*/
    drawCourse();
    
    break;

  case MENU_MAIN:
    /*Draw header*/
    drawHeader("MENU");
    /*Draw menu items*/
    noOfMenuItems = 0;
    drawMenuItem("START SESSION",MENU_SESSION_PLACEMENT);
    drawMenuItem("COURSE",MENU_COURSE_PLACEMENT);
    drawMenuItem("BUOYS",MENU_BUOYS_PLACEMENT);      
    drawMenuItem("SETTINGS",MENU_SETTINGS_PLACEMENT);
    /*Draw selected cursor*/
    drawCursor();
    break;

  case MENU_SESSION:
    /*Draw header*/
    drawHeader("SESSION");
    
    /*TODO Still to be determined*/
    display.setTextColor(BLACK);
    display.setTextSize(1);
    display.setCursor(1,18);
    display.println("Under\nConstruction");
    
    /*Draw selected cursor*/
    drawCursor();
    break;

  case MENU_COURSE:
    /*Draw header*/
    drawHeader("COURSE");

    /*Draw menu items*/
    noOfMenuItems = 0;
    drawMenuSliderItem("WIDTH","m",MENU_COURSE_WIDTH_PLACEMENT,courseWidth,COURSE_MIN_WIDTH,COURSE_MAX_WIDTH,0);
    drawMenuSliderItem("LENGTH","m",MENU_COURSE_LENGTH_PLACEMENT,courseLength,COURSE_MIN_LENGTH,COURSE_MAX_LENGTH,0);
    drawMenuSliderItem("TURNS","m",MENU_COURSE_TURNS_PLACEMENT,courseTurns,COURSE_MIN_TURNS,COURSE_MAX_TURNS,0);
    /*Draw selected cursor*/
    drawCursor();
    break;  
  
  case MENU_COURSE_WIDTH:
  case MENU_COURSE_LENGTH:
  case MENU_COURSE_TURNS:
    /*Draw header*/
    drawHeader("COURSE");
    /*Draw menu items*/
    noOfMenuItems = 0;

    /*Draw menu item*/
    if(menuState == MENU_COURSE_WIDTH)
      drawMenuSliderItem("WIDTH","m",0,courseWidth,COURSE_MIN_WIDTH,COURSE_MAX_WIDTH,1);
    else if(menuState == MENU_COURSE_LENGTH)
      drawMenuSliderItem("LENGTH","m",0,courseLength,COURSE_MIN_LENGTH,COURSE_MAX_LENGTH,1);
    else if(menuState == MENU_COURSE_TURNS)
      drawMenuSliderItem("TURNS","m",0,courseTurns,COURSE_MIN_TURNS,COURSE_MAX_TURNS,1);
    /*Draw Course*/
    drawCourse();
    break;

  case MENU_BUOYS:
    /*Draw header*/
    drawHeader("BUOYS");
    /*Draw menu items*/
    noOfMenuItems = 22;
    drawBuoySelectStatusCourse(innerMenuState);
    
    /*Draw selected cursor*/
    if(innerMenuState == MENU_BACK)
      drawCursor();
    break;
    
  case MENU_BUOY:
    /*Draw header*/
    drawHeader("DATA");
    /*Draw menu items*/
    noOfMenuItems = 0;
    char buf[4];
    drawStatusItem("Heading: ",itoa(buoyCompass,buf,10),0);
    drawStatusItem("No of Sats: ",itoa(buoyNoSat,buf,10),1);
    drawStatusItem("Lat: ",itoa(buoyLat,buf,10),2);
    drawStatusItem("Lon: ",itoa(buoyLon,buf,10),3);
    
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
      drawMenuSliderItem("CONTRAST","%",MENU_CONTRAST_PLACEMENT,contrast,0,100,1);
    else
      drawMenuSliderItem("CONTRAST","%",MENU_CONTRAST_PLACEMENT,contrast,0,100,0);
    /*Draw selected cursor*/
    drawCursor();
    break;


  } 

  /*Redraw Screen*/
  display.display();
}





/*Draws a menu item with a slider function, the menu item displays the current value when inactive.
 When active, the string is replaced by the slider which is then drawn as specified with min max and
 its current value*/
void drawMenuSliderItem(char* string,char* unit,short index,int value,int minValue,int maxValue,short active)
{

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
    display.drawRect(CURSOR_WIDTH+1,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER,SLIDER_BAR_LENGTH,LINE_HEIGHT,BLACK);
    display.fillRect(CURSOR_WIDTH+1,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER,(int)(((SLIDER_BAR_LENGTH*(value-minValue))/(maxValue-minValue))),LINE_HEIGHT,BLACK);
  }
  
  /*Make sure value is positioned against unit*/
  int offset = 0;
  if(value < 10)
    offset = CHAR_WIDTH;
    
  itoa(value,buffer,10);
  
  //Draw slider value
  display.setCursor(display.width() - 20+offset,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
  display.println(buffer);
  
  display.setCursor(display.width() - CHAR_WIDTH,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
  display.println(unit);

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

/*Draws a status item under the header location with a 
 given index. This allows menu items to be easily swapped
 in the configuration. Menu items should have names 6 
 characters or less, its assumed that the 0 state is the
 root state*/
void drawStatusItem(char* string,char* value,short index)
{
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(CURSOR_WIDTH+1,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
  display.println(string);
  display.setCursor(display.width()-strlen(value)*CHAR_WIDTH,index*(LINE_HEIGHT + LINE_SPACER)+HEADER_HEIGHT+ LINE_SPACER);
  display.println(value);

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

  int lengthOfString = strlen(string);
  int endOfLine = display.width(); 

  if(menuState != MENU_STATUS)
  {
    display.setCursor(BACK_BUTTON_START + BACK_BUTTON_CURSOR_WIDTH + 1,0);
    display.println("Back");

    if (innerMenuState == MENU_BACK)
      endOfLine  = BACK_BUTTON_START -1 - 1;
    else
      endOfLine  = BACK_BUTTON_START -1 + BACK_BUTTON_CURSOR_WIDTH;
  }

  display.drawLine(lengthOfString*CHAR_WIDTH,(HEADER_HEIGHT- LINE_SPACER),endOfLine,(HEADER_HEIGHT)- LINE_SPACER,BLACK);
  display.drawLine(lengthOfString*CHAR_WIDTH,(HEADER_HEIGHT- LINE_SPACER)-1,endOfLine,(HEADER_HEIGHT)- LINE_SPACER-1,BLACK);


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

/*Initialise screen through with SPI bus with defined pins*/
void initScreen(int setMenuState, int setInnerMenuState)
{  
  /*Initialise screen*/
  display.begin();

  /*Set the contrast of the display*/
  display.setContrast(contrast);
  
  /*Initialise menu system*/
  menuState = setMenuState; //Set the menu to status
  innerMenuState = setInnerMenuState;
  noOfMenuItems = 0;
  screenChanged = 1;
}

/*Interrupt on encoder events*/
void timerIsr() {
  
  encoder->service();
}

/*Initialise encoder by setting up interrupts*/
void initEncoder()
{  
  /*Initialise interrupt to check button status*/
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
}


/*Draws the ski course in pixels on the screen with a given position*/
void drawCourse()
{
  
  int turns = courseTurns;
  int centreLine = display.height() - COURSE_DISPLAY_SIZE/2 -1;
  int middleCourse = 42;
  int turnSpacing = ((courseLength-COURSE_MIN_LENGTH)*((display.width()-2 - 2*(turns+2))/(turns+2)))/(COURSE_MAX_LENGTH - COURSE_MIN_LENGTH)+3;
  int outsideBuoy = ((courseWidth-COURSE_MIN_WIDTH)*(COURSE_DISPLAY_SIZE/2-1-5))/(COURSE_MAX_WIDTH - COURSE_MIN_WIDTH);
  int startX = middleCourse-(turnSpacing*(turns+2))/2+turnSpacing/2;

  //Draw frame with line at top and line at bottom
  display.drawLine(0,display.height() - COURSE_DISPLAY_SIZE,display.width(),display.height() - COURSE_DISPLAY_SIZE,BLACK);
  display.drawLine(0,display.height()-1,display.width(),display.height()-1,BLACK);

  //Draw Gates
  int i = 0;
  for (i = 0;i<turns +2;i++)
  {
    display.drawPixel(startX + turnSpacing*i,centreLine-1,BLACK);
    display.drawPixel(startX+ turnSpacing*i,centreLine+1,BLACK);
  }

  //Draw Turn Buoys
  for (i = 1;i<turns +1;i++)
  {
    if(i%2 == 0)
      display.fillCircle(startX + turnSpacing*i,centreLine-outsideBuoy-4,1,BLACK);
    else
      display.fillCircle(startX + turnSpacing*i,centreLine+outsideBuoy+4,1,BLACK);
  }

}

/*Draws the buoys on screen, showing the current selection as well as the availabillity of the buoy*/
void drawBuoySelectStatusCourse(int selection)
{
  
  int turns = courseTurns;
  int centreLine = display.height() - COURSE_DISPLAY_SIZE/2 -1-5;
  int middleCourse = 42;
  int turnSpacing = ((COURSE_MAX_LENGTH-COURSE_MIN_LENGTH)*((display.width()-2 - 2*(courseTurns+2))/(courseTurns+2)))/(COURSE_MAX_LENGTH - COURSE_MIN_LENGTH)+2;
  int outsideBuoy = 11;
  int startX = middleCourse-(turnSpacing*(courseTurns+2))/2+turnSpacing/2;

  byte buoyStatusCount = 0;
  //Draw Gates
  int i = 0;
  for (i = 0;i<courseTurns +2;i++)
  {
    if(buoyStatus[buoyStatusCount])
      display.fillCircle(startX + turnSpacing*i,centreLine-4,2,BLACK);
    else
      display.drawCircle(startX + turnSpacing*i,centreLine-4,2,BLACK);
    buoyStatusCount += 1;
    
    if(buoyStatus[buoyStatusCount])
      display.fillCircle(startX+ turnSpacing*i,centreLine+4,2,BLACK);
    else
      display.drawCircle(startX + turnSpacing*i,centreLine+4,2,BLACK);
    buoyStatusCount += 1;
  }
  
  
  //Draw Turn Buoys
  for (i = 1;i<courseTurns +1;i++)
  {
    if(i%2 == 0)
    {
      if(buoyStatus[buoyStatusCount])
        display.fillCircle(startX + turnSpacing*i,centreLine-outsideBuoy-4,2,BLACK);
      else
        display.drawCircle(startX + turnSpacing*i,centreLine-outsideBuoy-4,2,BLACK);
    }
    else
    {
      if(buoyStatus[buoyStatusCount])
        display.fillCircle(startX + turnSpacing*i,centreLine+outsideBuoy+4,2,BLACK);
      else
        display.drawCircle(startX + turnSpacing*i,centreLine+outsideBuoy+4,2,BLACK);
    }
  }
  
  //Draw cursor
  if(selection >= 0 && selection < (courseTurns+2)*2)
  {
    byte remainder = selection%2;
    if(remainder == 0)
      display.drawCircle(startX + turnSpacing*((selection-remainder)/2),centreLine-4,4,BLACK);
    else      
      display.drawCircle(startX + turnSpacing*((selection-remainder)/2),centreLine+4,4,BLACK);
  }
  if(selection > (courseTurns+1)*2+1)
  {
    byte remainder = selection%2;
    if(selection%2 == 0)
    {
        display.drawCircle(startX + turnSpacing*(1+selection-(courseTurns+2)*2),centreLine+outsideBuoy+4,4,BLACK);
    }
    else    {
        
        display.drawCircle(startX + turnSpacing*(1+selection-(courseTurns+2)*2),centreLine-outsideBuoy-4,4,BLACK);
    }
  }

}
