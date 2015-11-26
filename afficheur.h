/*
 gilles
*/ 
 


#ifndef afficheur_h
#define afficheur_h

#include <Arduino.h>
#include <Wire.h>

enum{
  AFFICHERIEN,
  AFFICHEUSB,
  AFFICHESWEEP,
  AFFICHEPOTAR,
  AFFICHEMILIEU,
  AFFICHEADAFRUIT,
  AFFICHECLASSIQUE
};

class afficheur
{
  public:
    afficheur(byte adresse);
    void refreshAfficheur();
    void affiche(byte toto); // toto correspond à une grille de correspondance cf la méthode
   
    
    
    
    

  private:
    
    void envoi(byte data);// envoi effectivement le byte toto sur le busI2C vers l'afficheur
    byte _adresse; //numéro sur bus I2C du byte expander
    byte _affichageEnCours[10]; //tableau de la séquence en cours (si siéquence non animé 1 seul byte dans le tableau)
    byte _numSeq; //numéro de l'octet affiché dans la sequence animé : donc=0 en permanence pour affichage non animé
    byte _numMax; //corespond au numéro de byte max dans le tableau affichageEnCours à chercher dans letableau lors d'une animation (0 dnas le cas d'un affichage non animé)

    };

#endif
