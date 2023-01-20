/*
 Name:		ESP32_IoT_Hub.ino
 Created:	1/20/2023 10:22:49 PM
 Author:	jiaji
*/

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	delay(1000);
	Serial.println("Hello World!");
}

// the loop function runs over and over again until power down or reset
void loop() {
	Serial.println("Loop");
}
