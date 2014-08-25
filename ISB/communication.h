#ifndef COMMUNICATION
#define COMMUNICATION

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

void checkIfPacketsAreAvailable();
void initRF();


#endif
