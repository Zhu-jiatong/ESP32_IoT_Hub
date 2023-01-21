/*
 Name:		ESP32_IoT_Hub.ino
 Created:	1/20/2023 10:22:49 PM
 Author:	jiaji
*/

#include "hardware.h"
#include "networking.h"

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	cst::begin_SD();
	cst::load_cfg();
	cst::begin_network();
	cst::begin_server();
}

// the loop function runs over and over again until power down or reset
void loop() {

}
