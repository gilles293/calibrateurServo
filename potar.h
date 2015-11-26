/*
 gilles
*/ 
 


#ifndef potar_h
#define potar_h

#include <Arduino.h>


class potar
{
  public:
    potar(byte pin);
    void refresh();
    bool hasBeenMovedALot();
    void acquit(); //remet les état à zéro (moved)
    int getValue();//retourne valeur reel du potar
    
    
    
    

  private:
    
    byte _pin; //numéro de Pin pour le potar
    int _value; // Valeur réelle
    int _valueRef; // valeure de reference pour voir s'il bouge
    bool _movedALot; // si a bougé beaucoups
    
    

    };

#endif
