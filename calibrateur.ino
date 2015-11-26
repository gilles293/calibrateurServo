#include <SCoop.h>
#include <Wire.h>
#include "bouton.h"
#include "afficheur.h"
#include "servoTest.h"
#include "potar.h"
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>
#define DELTA 150
#define TEMPOSLEEP 1000

bouton boutonP(4);
afficheur affichage(0x20);
byte etatCalibrateur (11); //1= PC 2=Sweep 3=potar 4=Milieu Par défaut = 3 au démarage
unsigned long temp;
potar lePotar(0);
servoTest leServo(5); 
servoTest potence(11);
unsigned long dateChangeServo;
bool changeTypeServo(false);
int compteur(0);
int compteurRef(0);


defineTimerRun(rafraichirAffichage,800) //ne pas mettre ce timer a une valeur trop faible pour éviter le rebond lors de l'apuui bouton (ne pas compter des apuis multiple pour un seul appuie voulu)   
        {  
         //Serial.println("affichagePP");
          affichage.refreshAfficheur();
         
          }

defineTimerRun(surveilleBouton,40) //ne pas mettre ce timer a une valeur trop faible pour éviter le rebond lors de l'apuui bouton (ne pas compter des apuis multiple pour un seul appuie voulu)   
        {  
         //Serial.println("bouton");
          boutonP.refreshBouton();
         
          }

defineTimerRun(surveilleServo,100) //conserver 100 ou modifier le "*0.1" dans méthode appliqueobjectif du Servo
        {  
         //Serial.println("Servo");
         //Serial.println("potence.
         leServo.appliqueObjectif();
         potence.appliqueObjectif();
         

         
          }


defineTimerRun(surveillePotar,80)
  {
   // Serial.println("potar");
    lePotar.refresh();
             
    
    }

defineTask(reflechi)
      
      
      void reflechi::setup()
      
          {
             
           }
    
    void reflechi::loop()
          

//defineTimerRun(reflechi,40)
  {
    //Serial.println("proute");
    //Serial.println("reflechi");
      if (boutonP.hasBeenLongClicked())
        { 
          
          dateChangeServo=(millis());
          changeTypeServo=true;
          
          if (leServo.getType())
              {
                leServo.setType(false);
                affichage.affiche(5);
                Serial.println("servo de type Adafruit");
                
                }
           else
             {
               leServo.setType(true);
               affichage.affiche(6);
               Serial.println("servo de type Classique");
               }
          //sleep( 3000);
          boutonP.acquit();
          }
          
           
           if (millis()>dateChangeServo+3000 && changeTypeServo)
          {
           affichage.affiche(etatCalibrateur);
           changeTypeServo=false;
            }
                 
          
          
          
          
    int objTest (0);
    
    switch (etatCalibrateur)
        {
        
          
          
          
          case 1:
             
             Serial.println("Double Clic pour procéder à l'etlonnage");
             etatCalibrateur=10;
             break;
             
             
                   
          case 10:
          
             if (boutonP.hasBeenClicked())
            {
              boutonP.acquit();
              etatCalibrateur=2;
              affichage.affiche(2);
              leServo.setObjectif(leServo.getMin());
                                          
              }
             
             if (boutonP.hasBeenDoubleClicked())
             {
               boutonP.acquit();
               Serial.println("tourner potar à fond à gauche");
               etatCalibrateur=11;
              } 
           
             break;   

          case 11:
              
              leServo.setObjectif(leServo.getMilieu());// pour etre certain qu'on aprt du milieu et que le servo n'est pas n'importe ou suite à des manip de type sweep ou potar par ex
              if (lePotar.getValue()>1000)
             {
               etatCalibrateur=12;
               Serial.println("quand potence reglé (sur le disque optique) faire 1 clic");
                              
              }   
                 
             break;   
            
             
            
        case 12 :
   
//           Serial.print("obj potence=");
//           Serial.print(potence.getObjectif());
//           Serial.print("   Encours potence=");
//           Serial.print(potence.getEnCours());
//           Serial.print("   obj Pot=");
//           Serial.println(map(lePotar.getValue(),0,1023,potence.getMin(),potence.getMax()));
          // Serial.println("_____");
           
           
           potence.setObjectif(map(lePotar.getValue(),0,1023,potence.getMin(),potence.getMax()));
           if (boutonP.hasBeenClicked())
            {
              boutonP.acquit();
              etatCalibrateur=13;
              }
           
           
           
             break;
         
         case 13:
             Serial.println("ICI");
             compteur=0;
             compteurRef=0;
             leServo.setObjectif(leServo.getMilieu()+DELTA);
            
             sleep(TEMPOSLEEP);
             etatCalibrateur=14;
             Serial.println("la");
             

             
             break;
        case 14: 
             Serial.println("lalalal");
             
               compteurRef=compteur;
//             objTest=leServo.getMilieu();
//             while (compteur>compteurRef-3)
//               {
//                 Serial.println("LA");
//                 compteur=0;
//                leServo.setObjectif(objTest+DELTA);
//                 sleep(TEMPOSLEEP);
//                 objTest=objTest+DELTA;
//                 }
//             leServo.setMax(objTest-DELTA);
//             leServo.setObjectif(leServo.getMilieu());
             
             
             break;
                  
        case 2:
             if (leServo.isMin())
            {
              
              etatCalibrateur=20;
              leServo.setObjectif(leServo.getMax());
                                         
              }
              
              if (boutonP.hasBeenClicked())
            {
              boutonP.acquit();
              etatCalibrateur=3;
              affichage.affiche(3);
              }
                 
             break;
    
    case 20:
             if (leServo.isMax())
            {
              
              etatCalibrateur=2;
              leServo.setObjectif(leServo.getMin());
                                         
              }
               
               if (boutonP.hasBeenClicked())
            {
              boutonP.acquit();
              etatCalibrateur=3;
              affichage.affiche(3);
              }
               
               
                 
             break;
    
    
             
            
        case 3:
          int temp;
          int potarVal;
          potarVal=lePotar.getValue();
          //Serial.print("lepotar : ");
          //Serial.println(potarVal);
          temp=map(potarVal,0,1023,leServo.getMin(),leServo.getMax());
          
          
//          if (leServo.getType())
//          
//            {
//              temp=map(potarVal,0,1023,300,3000);
//            }
//          else
//            {
//             temp=map(potarVal,0,1023,100,1000);
//              }
              
           //Serial.print ("OBJECTIF=  ");
           //Serial.println(temp);   
           leServo.setObjectif(temp);
           //potence.setObjectif(temp);

          if (boutonP.hasBeenClicked())
            {
              boutonP.acquit();
              etatCalibrateur=4;
              affichage.affiche(4);
              //leServo.setObjectif(leServo.getMilieu());
              
              }
          
          
          
          break;
    
      case 4:
           
           leServo.setObjectif(leServo.getMilieu());
           if (boutonP.hasBeenClicked())
            {
              boutonP.acquit();
              etatCalibrateur=1;
              affichage.affiche(1);
                            
              }
           
           
           
           
           break;
           
      case 8:
        
        break;
        
        
        
      }

    
    
    
    }


          
void setup() 
   {     
     Serial.begin(57600);
     potence.setPotence(); 
     leServo.setType(true); 
     Wire.begin();
    
    Serial.println("hello");
    mySCoop.start();
   temp=millis();
   //etatCalibrateur=3; fait dans l'init de la variable
   affichage.affiche(3);
   attachInterrupt(0, updatePulseCompteur, CHANGE ); // Int0= la pin N°2 sur un UNO
   
     
     }
 void updatePulseCompteur()
    { 
      compteur++;       
    }     
         
 void loop ()
 
   {
     mySCoop.sleep(1);
    
//    if (boutonP.hasBeenClicked()==true)
//      {
//        Serial.println("clic ");
//        boutonP.acquit();
//        
//        }
//   if (boutonP.hasBeenDoubleClicked()==true)
//      {
//        Serial.println("DoubleClic ");
//        boutonP.acquit();
//        
//        }    
//        
//    if (boutonP.hasBeenLongClicked()==true)
//      {
//        Serial.println("Long Clic ");
//        boutonP.acquit();
//        
//        }  
    
    
    if (millis()>2000+temp)
         {
         
         //Serial.print("valeur potar : ");
         //Serial.println(lePotar.getValue());
         temp=millis();
         
         //Serial.print("Etat : ");
    //Serial.println(etatCalibrateur);
         }
         
     }

