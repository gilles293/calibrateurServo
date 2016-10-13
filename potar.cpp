/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
# 
# Classe potar:

#*******************************************************************************
 J.Soranzo
    13/10/2016: commentaire
*/ 
#include "potar.h"

//------------------------------------------------------------------------------
// Constructeur
potar::potar(byte pin)
{
    _pin=pin;
    _value=0;
    _value=_valueRef;
    _movedALot=false; 
}

//------------------------------------------------------------------------------
// Methodes publiques
int potar::getValue(){ return _value; }    
bool potar::hasBeenMovedALot(){ return _movedALot; }

//------------------------------------------------------------------------------
// Methodes publiques
void potar::init()
{
    _valueRef=analogRead(_pin);
    _value=_valueRef; 
}

// Methode qui doit être appelee periodiquement
// Par exemple toutes les 80ms a gerer par un timer externe 
// cette methode filtre les petite variations
// jusqu'a ce qu'elles soient suffisement significatives >100   
void potar::refresh()
{
    //Serial.print("value=");Serial.println(_value);
    //Serial.print("valueRef=");Serial.println(_valueRef);
    // equ !( abs( _value - analogRead() ) > 10 )
    if ( !(_value-10<analogRead(_pin) && analogRead(_pin)<_value+10) )
    {
        _value=analogRead(_pin);
    }
    // le _movedALot est tres probablement inutile
    if (_movedALot||_value>_valueRef+100 || _value<_valueRef-100)
    {
      _movedALot=true;
      //Serial.println("movedAlot");
    }
}
      
 void potar::acquit()
{
    _valueRef=analogRead(_pin);
    _movedALot=false;
}
  

  
 
    
