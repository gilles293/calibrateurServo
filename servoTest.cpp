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
#include "servoTest.h"

//constructeur
servoTest::servoTest(byte pin)
    {
      _pin=pin;
      _carteAda = Adafruit_PWMServoDriver(0x40); 
    }
 
// Les getteurs 
int servoTest::getObjectif(){ return _objectif; }   
int servoTest::getEnCours() { return _enCours;  } 
bool servoTest::getType()   { return _type;     }
int servoTest::getMilieu()  { return _milieu;   }
int servoTest::getMin()     { return _min;      }
int servoTest::getMax()     { return _max;      }

// les setteurs
void servoTest::setMax(int val)      { _max=val;      }
void servoTest::setMin(int val)      { _min=val;      }
void servoTest::setObjectif(int val) { _objectif=val; }
void servoTest::setVitesse(int val)  { _vitesse=val;//Serial.print("vitesse=");Serial.println(val); 
} 

//autres méthodes
bool servoTest::isMin()         { return(_enCours==_min); } 
bool servoTest::isMax()         { return(_enCours==_max); }

// si type = true => servo pilote directement par la carte ARDUINO
void servoTest::setType(bool type)
  {
    _type=type;
	
    if (type)
		{
			_min=500;   //1000
			_max=2500; //2000 en principe mais la référence Arduino
     // dit que certains fabricant ne respecte pas ces valeurs...
			_milieu=(_min+_max)/2;
			_vitesse=300; //300 par defaut JSO : reolution de conflit 3000 to 300
			_enCours=_milieu+1;
			_objectif=_milieu; 
			_myServo.attach(_pin);       
		}
		
    else
       {
			_min=300;
			_max=400;
			_milieu=(_min+_max)/2;
			_vitesse=50; //JSO : resolution de conflit 3000 to 50
			_enCours=_milieu+1;
			_objectif=_milieu;    
			Serial.println("hehe");
			_carteAda.begin();
			_carteAda.setPWMFreq(50); 
		}
		
}
    

// C'est reellement cette methode qui provoque les deplacement du servomoteur    
void servoTest::setEnCours(int val)
{  
//   Serial.print(" val=");
//   Serial.println(val);
//   Serial.print(" pin=");
//   Serial.println(_pin);
   if (_enCours!=val)
        {
			_enCours=val;
			if (_type==false)
				{
					_carteAda.setPWM(0, 0, val);
				}
            else
				{
				   // Serial.println(val);
					_myServo.writeMicroseconds(val);
				}
		}
}

void servoTest::appliqueObjectif()
{
   float pas(0);
   pas=(float)_vitesse*0.1;
//  Serial.print("obj= ");
//  Serial.println(_objectif);
//  Serial.print("en cours =");
//  Serial.println(_enCours);
    if (_objectif>_enCours)
		{
			if (pas>(_objectif-_enCours))
				{
					setEnCours(_objectif);
				}
			 else
				{
					setEnCours(_enCours+pas);
				}
		}
     else if (_objectif<_enCours)
        {
			if (pas>(_enCours-_objectif))
				{ 
					setEnCours(_objectif);
				}
			else
				{
					setEnCours(_enCours-pas);
				}
        }
 
   }
