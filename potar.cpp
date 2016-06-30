
#include "potar.h"

potar::potar(byte pin)
    {
      _pin=pin;
      _value=0;
      _value=_valueRef;
      _movedALot=false;
      
      
      
    }

void potar::init()

{
_valueRef=analogRead(_pin);
_value=_valueRef;
  
}

    
    
bool potar::hasBeenMovedALot()
  {
    
    return _movedALot;
    
    
    }
    
void potar::refresh ()

{
 // Serial.print("value=");Serial.println(_value);
  //Serial.print("valueRef=");Serial.println(_valueRef);
  
  if (_value-10<analogRead(_pin) && analogRead(_pin)<_value+10)
    {
      
      }
    
    else
    {
      _value=analogRead(_pin);
      }
  
  
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
  
  int potar::getValue()
  
  {
    return _value;
    
    
   }
  
 
    
