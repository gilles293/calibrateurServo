/*
#*******************************************************************************
# Projet : VOR005
# Sous projet : calibrateur de servo moteurs
# 
# Auteurs: Gille De Bussy
# 
# class afficheur:
# gestion afficheur 7 segments au travers d'un PCF8574 en I2C
#*******************************************************************************
 J.Soranzo
	07/11/2015 gestion sous git + github
	https://github.com/MajorLee95/VOR005_calibrateurServoSoft.git
*/
//test de chaque segment de led    
//        _affichageEnCours[0]=0xFD;//
//        _affichageEnCours[1]=0xFB;
//        _affichageEnCours[2]=0xF7;
//        _affichageEnCours[3]=0xEF;
//        _affichageEnCours[4]=0xDF;
//        _affichageEnCours[5]=0xBF;
//        _affichageEnCours[6]=0xFE;
//        _affichageEnCours[7]=0xFE;


#include "afficheur.h"

/*
// J.Soranzo : afficheur monté à l'envers !
P : 0x98
O :0x01
T : 0xF0
A: 0x88
R : 0xFA
d:0xC2
f:0xB8
U : 0xC1
I: 0xCF
M :0x89
N
S : 0xA4
b : 0xF0
E : 0xB0
L : 0XF1
w: 0xE3
*/

afficheur::afficheur(byte adresse)
    {
    _adresse=adresse; 
    _affichageEnCours[0]=0; 
    _numSeq=0; 
    _numMax =0;
      
    }
    
    
    
void afficheur::refreshAfficheur()
{
 // Serial.print("numSeq ");
  //Serial.println(_numSeq);
  
  if (_numMax==0) return;
  
  _numSeq=_numSeq+1;
  if (_numSeq>_numMax)
    {
      _numSeq=-1;
      envoi(0xFF);
    }
    else
    {
       envoi(_affichageEnCours[_numSeq]);
    }
}

void afficheur::envoi(byte data)
{
  //Serial.println(_adresse,HEX);
  Wire.beginTransmission(_adresse);
  //Serial.println(data,HEX);
  Wire.write(data);
  //Serial.println("ta");
  int a=Wire.endTransmission(); 
  //Serial.println(a)  ;
  
  
}

void afficheur::affiche(byte choixAffichage)
{
  switch (choixAffichage)
    {
    case AFFICHERIEN : // affiche rien
        _affichageEnCours[0]=0xFF;
       Serial.println(F("Rien")); 
        _numMax =0;       
        break;
        
    case AFFICHEUSB : // affiche USB 
        _affichageEnCours[0]=0x89;//
        _affichageEnCours[1]=0xA4;//
        _affichageEnCours[2]=0x8C;//
        
       Serial.println(F("Mode : USB")); 
         
        _numMax =2;       
        break;
  
    case AFFICHESWEEP : // affiche SWEEP 
        _affichageEnCours[0]=0xA4;//
        _affichageEnCours[1]=0x9D;//
        _affichageEnCours[2]=0x86;
        _affichageEnCours[3]=0x86;
        _affichageEnCours[4]=0xC2; 
       Serial.println(F("Mode : SWEEP")); 
        _numMax =4;       
        break;
     
     case AFFICHEPOTAR : 
        _affichageEnCours[0]=0xC2;
        _affichageEnCours[1]=0x81;
        _affichageEnCours[2]=0x8E;
        _affichageEnCours[3]=0xC0;
        _affichageEnCours[4]=0xDE;
        _numMax=4;
       Serial.println("Mode Potar"); 
               
        break; 
     
     
     case AFFICHEMILIEU : // affiche Milieu 
  
        _affichageEnCours[0]=0xC1;
        _affichageEnCours[1]=0xF9;//
        _affichageEnCours[2]=0x8F;//
        _affichageEnCours[3]=0xF9;
        _affichageEnCours[4]=0x86;//
        _affichageEnCours[5]=0x89;//
       Serial.println("Retour au MILIEU"); 
        
        _numMax =5;       
        break;    

        case AFFICHEADAFRUIT : // affiche A Adafruit 
        _affichageEnCours[0]=0xC0;
        _affichageEnCours[1]=0x98;
        _affichageEnCours[2]=0xC0;
        _affichageEnCours[3]=0xC3;
        _affichageEnCours[4]=0xDE;
        _affichageEnCours[5]=0x89;
        _affichageEnCours[6]=0xF9;
        _affichageEnCours[7]=0x8E;
       Serial.println("servo de type Adafruit"); 
        _numMax =7;       
        break;   
      
      
      case AFFICHECLASSIQUE : // affiche classique 
        _affichageEnCours[0]=0x87; 
        _affichageEnCours[1]=0x8F; 
        _affichageEnCours[2]=0xC0; 
        _affichageEnCours[3]=0xA4; 
        _affichageEnCours[4]=0xA4; 
        _affichageEnCours[5]=0xF9; 
        _affichageEnCours[6]=0xE0; 
        _affichageEnCours[7]=0x89; 
        _affichageEnCours[8]=0x86; 
       Serial.println("servo de type classique"); 
        
        _numMax =8;
           
        break;  

		
      
      
    }  
     _numSeq=0;
   // Serial.println(_affichageEnCours[0]);
     envoi(_affichageEnCours[0]);
    
  
  }
