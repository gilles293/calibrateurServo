/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
# 
# Classe potar: fichier d'entete

Classe qui s'utilise avec un timer
Elle filtre les faibles variations
jusqu'a ce qu'elles soient suffisement significatives

#*******************************************************************************
 J.Soranzo
    13/10/2016: commentaire
*/


#ifndef potar_h
#define potar_h

#include <Arduino.h>


class potar
{
  public:
    potar(byte pin);
    void init();
    //parce que dans le constructeur lire la pin anlogique
    //ne marche pas (=0sytématiquement)
    void refresh();
    bool hasBeenMovedALot();
    void acquit(); //remet les etat a zero (moved)
    int getValue();//retourne valeur reel du potar

  private: 
    byte _pin; //numero de Pin pour le potar
    int _value; // Valeur reelle
    int _valueRef; // valeure de reference pour voir s'il bouge
    bool _movedALot; // si a bouge beaucoups

};

#endif
