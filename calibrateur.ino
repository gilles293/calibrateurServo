/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
#
# class servo:
# permet de piloter les servo qui sont utilises sur le proet
# servo en test mais aussi servo de la potence
# utilise la librairie servo de chez Adafruit pour la carte
Adafruit 16-Channel 12-bit PWM/Servo Driver - I2C interface - PCA9685
PRODUCT ID: 815
#*******************************************************************************
 J.Soranzo
	26/11/2015 gestion sous git + github
	https://github.com/gilles293/calibrateurServo.git
*/
#include <Arduino.h>

/*
  Turns on an LED on for one second, then off for one second, repeatedly.
*/

void setup()
{
	Serial.begin(9600);
	Serial.println(F("Test fourche optique"));
	// initialize the digital pin as an output.
	// Pin 13 has an LED connected on most Arduino boards:
	pinMode(13, OUTPUT);
}

void loop()
{


}
