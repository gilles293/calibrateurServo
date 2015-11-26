/*
 gilles
*/ 
 


#ifndef bouton_h
#define bouton_h

#include <Arduino.h>


class bouton
{
  public:
    bouton(byte boutonPin);
    void refreshBouton();
    bool hasBeenClicked();
    bool hasBeenDoubleClicked();
    bool hasBeenLongClicked();
    void acquit(); //remet les état à zéro (clique et double clique)
    
    
    
    

  private:
    
    byte _boutonPin; //numéro de Pin pour le bouton
    unsigned long _finDernierAppui; // heure de la dernière fois que qq1 a relaché le bouton
    unsigned long _debutDernierAppui; // heure de la dernière fois que qq1 a appuyé le bouton
    byte _etat; // etat interne du bouton pour déterminer clique et double clique
    bool _clicked; // si a été cliqué
    bool _doubleClicked; // si a été doublecliqué
    bool _longClicked; //si subit clique long
    

    };

#endif
