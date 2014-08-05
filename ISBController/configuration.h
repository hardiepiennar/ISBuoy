#ifndef CONFIGURATION
#define CONFIGURATION

/*Menu layout definitions*/
#define LINE_HEIGHT 7
#define LINE_SPACER 2
#define HEADER_HEIGHT 11
#define BACK_BUTTON_START 51
#define BACK_BUTTON_CURSOR_WIDTH 8
#define CURSOR_WIDTH 3

/*Define menu states*/
#define MENU_STATUS 0
#define MENU_MAIN 1
#define MENU_BACK -1
//Main menu state
#define MENU_SESSION 2
#define MENU_COURSE 3
#define MENU_BOUYS 4
#define MENU_SETTINGS 5
//Settings state
#define MENU_SETTINGS_CONTRAST 6


/*Define menu orders*/
/*Main Menu*/
#define MENU_SESSION_PLACEMENT 0
#define MENU_COURSE_PLACEMENT 1
#define MENU_BOUYS_PLACEMENT 2
#define MENU_SETTINGS_PLACEMENT 3
/*Settings*/
#define MENU_CONTRAST_PLACEMENT 0

/*Define encoder settings*/
#define ENCODER_STEPS_PER_NOTCH 4
 
#endif
