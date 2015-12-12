#include <Arduino.h>

/*
  Turns on an LED on for one second, then off for one second, repeatedly.
*/

void setup()
{
	Serial.begin(9600);
	Serial.println(F("Hello world!"));
	// initialize the digital pin as an output.
	// Pin 13 has an LED connected on most Arduino boards:
	pinMode(13, OUTPUT);
}

void loop()
{


}
