#ifndef CONFIGURATION
#define CONFIGURATION

/*Menu layout definitions*/
#define LINE_HEIGHT 7
#define CHAR_WIDTH 6
#define LINE_SPACER 2
#define HEADER_HEIGHT 8
#define BACK_BUTTON_START 55
#define BACK_BUTTON_CURSOR_WIDTH 4
#define CURSOR_WIDTH 3
#define SLIDER_BAR_LENGTH 56
#define COURSE_DISPLAY_SIZE 29 //Keep even

/*Define menu states*/
#define MENU_STATUS 0
#define MENU_MAIN 1
#define MENU_BACK -1
//Main menu state
#define MENU_SESSION 2
#define MENU_COURSE 3
#define MENU_BUOYS 4
#define MENU_SETTINGS 5
/*Settings state*/
#define MENU_SETTINGS_CONTRAST 6
/*Course State*/
#define MENU_COURSE_WIDTH 7
#define MENU_COURSE_LENGTH 8
#define MENU_COURSE_TURNS 9


/*Define menu orders*/
/*Main Menu*/
#define MENU_SESSION_PLACEMENT 0
#define MENU_COURSE_PLACEMENT 1
#define MENU_BUOYS_PLACEMENT 2
#define MENU_SETTINGS_PLACEMENT 3
/*Course*/
#define MENU_COURSE_WIDTH_PLACEMENT 0
#define MENU_COURSE_LENGTH_PLACEMENT 1
#define MENU_COURSE_TURNS_PLACEMENT 2
/*Settings*/
#define MENU_CONTRAST_PLACEMENT 0

/*Define encoder settings*/
#define ENCODER_STEPS_PER_NOTCH 4

/*Define Generals settings*/
//Course dimensions
#define COURSE_MAX_WIDTH 15
#define COURSE_MIN_WIDTH 4
#define COURSE_MAX_LENGTH 35
#define COURSE_MIN_LENGTH 15
#define COURSE_MAX_TURNS 6
#define COURSE_MIN_TURNS 1
//Contrast setting
#define CONTRAST_MAX 80
#define CONTRAST_MIN 30

#endif
