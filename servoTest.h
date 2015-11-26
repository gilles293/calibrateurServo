/**
#*******************************************************************************
# Projet : VOR005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
# 
# class afficheur:
# gestion afficheur 7 segments au travers d'un PCF8574 en I2C
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
    servoTest(byte pin);
    bool getType();
    void setType(bool type);
    void setPotence();
    void setObjectif(int value);
    int getEnCours();
    int getMilieu();
    
    void setMax(int val);
    void setMin(int val);
    int getMin();
    int getMax();
    bool isMin();
    bool isMax();
    int getObjectif();
    int getVitesse();
    void setVitesse(int vit); // en étendue de microsec ou pwm par seconde
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
