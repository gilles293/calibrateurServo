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
 


#ifndef servoTest_h
#define servoTest_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>

class servoTest
{
  public:
    //contructor
	servoTest(byte pin);
	
	//setters & getters
    bool getType();
    void setType(bool type);
    void setPotence();
    void setObjectif(int value);
    int getObjectif();
    int getVitesse();
    void setVitesse(int vit); // en étendue de microsec ou pwm par seconde
    int getEnCours();
    int getMilieu();
    void setMax(int val);
    void setMin(int val);
    int getMin();
    int getMax();
	
	//autres membres publics
    bool isMin();
    bool isMax();
    void appliqueObjectif(); // methode à activer régilierment pour s'approcher de l'objectif à la vitesse choisie

  private:
    void setEnCours(int val);
    byte _pin;
    byte _type; ///true=classique false=Adafruit
    int _enCours; // valeur en cours
    int _objectif; //valeur à atteindre
    int _min;
    int _max;
    int _milieu;
    int _vitesse;
    Servo _myServo;
    Adafruit_PWMServoDriver _carteAda;
    };

#endif
