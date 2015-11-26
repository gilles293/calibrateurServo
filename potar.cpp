
#include "potar.h"

potar::potar(byte pin)
    {
      _pin=pin;
      _valueRef=analogRead(_pin);
      _value=_valueRef;
      _movedALot=false;
      
      
    }
    
    
bool potar::hasBeenMovedALot()
  {
    
    return _movedALot;
    
    
    }
    
void potar::refresh ()

{
  
  if (_value-10<analogRead(_pin) && analogRead(_pin)<_value+10)
    {
      
      }
    
    else
    {
      _value=analogRead(_pin);
      }
  
  
  if (_value>_valueRef+100 || _value<_valueRef-100)
    {
      _movedALot=true;
      }
      
  }
      
 void potar::acquit()
  {
    _valueRef=analogRead(_pin);
    _movedALot=false;
  
  }
  
  int potar::getValue()
  
  {
    return _value;
    
    
   }
  
 
    
