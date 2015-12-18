/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: J.Soranzo
#
# Programme principal: test de la fourche optique
# permet de piloter les servo qui sont utilises sur le proet
# servo en test mais aussi servo de la potence
# utilise la librairie servo de chez Adafruit pour la carte
#*******************************************************************************
 J.Soranzo
	17/12/2015 gestion sous git + github
	https://github.com/gilles293/calibrateurServo.git
*/
#include <Arduino.h>
#include <Servo.h>

#define SERVOPIN 5
#define CAPTEURINTERRUPNUMBER 0

Servo serv;

boolean switchServo = false;
boolean switchMiddle = false;
int compteur(0);

void updatePulseCompteur(){
    compteur++;
}

void setup()
{
	Serial.begin(9600);
	Serial.println(F("Test fourche optique 2000"));
	Serial.println("enter pour d�marrer la mesure et le servo");
	serv.attach(SERVOPIN);
    attachInterrupt(CAPTEURINTERRUPNUMBER, updatePulseCompteur, CHANGE );
}

void serialEvent(){
    while (Serial.available()) {
        Serial.read();
    }
    switchServo = !switchServo;


}


void loop()
{
//  for (int pos = 10; pos <= 170; pos += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    serv.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(10);                       // waits 15ms for the servo to reach the position
//  }
//  for (int pos = 170; pos >= 10; pos -= 1) { // goes from 180 degrees to 0 degrees
//    serv.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(10);                       // waits 15ms for the servo to reach the position
//  }
    if (switchServo){
        serv.write(175);              // tell servo to go to position in variable 'pos'
        delay(1500);
        serv.write(10);              // tell servo to go to position in variable 'pos'
        delay(1500);
        switchMiddle = true;
    } else if (switchMiddle){
        switchMiddle = false;
        serv.write(90);
    }

}
