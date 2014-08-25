#ifndef RADIO
#define RADIO
#include <arduino.h>

/*Radio General Settings*/
#define PAYLOAD_SIZE 8
#define MAX_QUEUE_SIZE 20

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

void updateNetwork();
void checkIfPacketsAreAvailable();
void checkBuoy(int buoyAddress);
void getBuoyCompass(int buoyAddress);
void requestBattery(int buoyAddress);
void requestLat(int buoyAddress);
void requestLon(int buoyAddress);
void initRF();

#endif
