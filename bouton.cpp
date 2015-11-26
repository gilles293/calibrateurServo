

#include "bouton.h"

bouton::bouton(byte boutonPin)
    {
      _boutonPin=boutonPin;
      _finDernierAppui=millis()+1;
      _debutDernierAppui=millis();
      pinMode(boutonPin, INPUT);  
      _etat=0;
      _doubleClicked=false;
      _clicked=false;
      _longClicked=false;
      
    }
    
    
bool bouton::hasBeenLongClicked()
  {
    
    return _longClicked;
    
    
    }
    
bool bouton::hasBeenClicked()
  {
    
    return _clicked;
    
    
    }
    
bool bouton::hasBeenDoubleClicked()
  {
    return _doubleClicked;
    }  
    
void bouton::acquit()

  {
    _doubleClicked=false;
    _clicked=false;
    _longClicked=false;
    
    
    }
    
void bouton::refreshBouton()
  {
    switch (_etat)
    
      {
        case 0:
          
           //Serial.println("case0");
          if (digitalRead(_boutonPin)==LOW)
            {
            //Serial.println("case0if");
              _etat=1;
              _debutDernierAppui=millis();
              break;
              }
            break;
         case 1:
            //Serial.println("case1");
           if (digitalRead(_boutonPin)==HIGH && millis()<_debutDernierAppui+800)//detection simple clique ou double clique
            {
              //Serial.println("case1if");
              _etat=2;
              _finDernierAppui=millis();
              break;
              }
              if (digitalRead(_boutonPin)==HIGH && millis()>=_debutDernierAppui+800)//detection clique long
            {
              
              _etat=0;
              _longClicked=true;
              Serial.println("long clic");
              break;
              }
              
              break;
              
         case 2:
          //Serial.println("case2");
          if (digitalRead(_boutonPin)==HIGH && millis()>_finDernierAppui+200)
            {
              Serial.println("CLIC");
              _etat=0;
              _clicked=true;
              break;
              }
           if (digitalRead(_boutonPin)==LOW && millis()<_finDernierAppui+200)
           
             {
               
              _etat=5;
              //Serial.println("etat5");
              break;
               }
          break;


         case 5: 
             if (digitalRead(_boutonPin)==HIGH )
           
             {
               Serial.println("double clique");
              _etat=0;
              _doubleClicked=true;
              break;
               }
             break;
         
         }
    
    
    
 
  }
    
  
