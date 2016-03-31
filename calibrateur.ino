/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
# 
# Program principal:

// A FAIRE : gérer les vitesse : les mettre à fond quand test usb les mettre plus bas pour les autres modes.
// A FAIRE : au démarage de l'arduino initialiser les valeurs potence loin de disque optique.
// enregistrer la potente en EEPROM

#*******************************************************************************
 J.Soranzo
	16/12/2015 gestion sous git + github
	https://github.com/gilles293/calibrateurServo.git
	nettoyage du code, j'ai craqué
*/ 

//pour la suite faire moyenne pour mesure PWMIN et PWMMAX et virer le calcul de la moyenne mesureimpulsion pour le remplacer par la méthode


#include <EEPROM.h>
#include <SCoop.h>
#include <Wire.h>
#include "bouton.h"
#include "afficheur.h"
#include "servoTest.h"
#include "potar.h"
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>
#define DELTA 150
#define TEMPOSLEEP 800 //en ms le temps pour le servo de bouger d'un increment quand on fait les mesure à la roue codeuse
#define ERREURCOMPTAGE 20 //la tolérance de comptage avant que l'on estime que le servo est arrivé au bout
#define NBRCYCLE_AR 10 //nbre de cycle d'aller et retour pour la fin des mesure de caractérisation et nbr de cycle pour la mesure des PWMIN et PWMMAX
#define TEMPO_STAT 200 // tempo pour laisser le servo avancr lors des mesures statistique à la fin de la calibration
#define ADRESSE_EEPROM 42//position de leeprom ou la valeur de reglage de l potence est stocké

enum {
		POTAR = 3,
		MILIEU = 4,
		USB = 1,
		ETALORSWEEP = 10,
		REGPOTENCINVIT = 11,
		REGLPOTENC = 12,
		MESREFPULSE = 13,
		FINDMINMAX = 14,
		WAITCLIC = 15,
		DISPRESULT = 16,
		WAITCLIC2 = 17,
    LECTURE_OU_ECRITURE_EEEPROM=18,
		SWEEPTOMIN = 2,
		SWEPPTOMAX = 20
};
bouton boutonP(4);
afficheur affichage(0x20);
int bonnePositionPotence;
byte etatCalibrateur (POTAR); //1= PC 2=Sweep 3=potar 4=Milieu Par défaut = 3 au démarage
unsigned long temp;
unsigned long tempsMesureVitesse;
potar lePotar(0);
servoTest leServo(5); 
servoTest potence(11);
unsigned long dateChangeServo;
bool changeTypeServo(false);
int compteur(0);
int compteurRef(0);
int resultatImpulsion[NBRCYCLE_AR*2];
long resultatTemps[NBRCYCLE_AR*2];
int mesureRef[NBRCYCLE_AR];
int PWMMin[NBRCYCLE_AR];
int PWMMax[NBRCYCLE_AR];


int objTest (0); // varaible temporaire pour ...peut sans doute etre supprimé en faisant apelle aux bnnes méthode pour connaitre etat courrant du servo

int moyenne (int tailleTab, int tab [])

{
  byte j;
  long moyenne (0);
  for (j=0;j<tailleTab;j++)
         {
              moyenne=moyenne+tab[j];
          }
   moyenne=moyenne/(tailleTab);
   return moyenne;
                            
}


defineTimerRun(rafraichirAffichage,800)
 //ne pas mettre ce timer a une valeur trop faible pour éviter le rebond
 //lors de l'apuui bouton (ne pas compter des apuis multiple pour un seul appuie voulu)   
{  
	 //Serial.println("affichagePP");
	  affichage.refreshAfficheur();
}

defineTimerRun(surveilleBouton,40)
//ne pas mettre ce timer a une valeur trop faible pour éviter le rebond lors de
//l'apuui bouton (ne pas compter des apuis multiple pour un seul appuie voulu)   
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
    byte i;
		if (boutonP.hasBeenLongClicked())
			{ 
				dateChangeServo=(millis());
				changeTypeServo=true;
				if (leServo.getType())
					{
						leServo.setType(false);
						affichage.affiche(5);
						Serial.println(F("servo de type Adafruit"));
					}
				else
					{
						leServo.setType(true);
						affichage.affiche(6);
						Serial.println(F("servo de type Classique"));
					}
				//sleep( 3000);
				boutonP.acquit();
			}
		if (millis()>dateChangeServo+3000 && changeTypeServo)
			{
				affichage.affiche(etatCalibrateur);
				changeTypeServo=false;
			}

		
		switch (etatCalibrateur)
			{
				case USB :
					
                                        
                                        Serial.println(F("Double Clic pour procéder à mesure via USB, sinon simple clic change de mode"));
                                        
					etatCalibrateur=ETALORSWEEP;
					break;

				case ETALORSWEEP :
					if (boutonP.hasBeenClicked())
						{
							boutonP.acquit();
							etatCalibrateur=SWEEPTOMIN;
							affichage.affiche(SWEEPTOMIN);
							leServo.setObjectif(leServo.getMin());
						}
				 
					if (boutonP.hasBeenDoubleClicked())
					{
						boutonP.acquit();
            leServo.setVitesse(4000);
            leServo.setObjectif(leServo.getMilieu());
         // pour etre certain qu'on aprt du milieu et que le servo 
          //n'est pas n'importe ou suite à des manip de type sweep
          //ou potar par ex
						
						Serial.println(F("si lecture EEPROM pour potence Simple clic, si reglage potence double clic"));
						etatCalibrateur=LECTURE_OU_ECRITURE_EEEPROM;
					} 
					break;   

				case LECTURE_OU_ECRITURE_EEEPROM :
					if (boutonP.hasBeenClicked())
						{
							boutonP.acquit();
							EEPROM.get(ADRESSE_EEPROM, bonnePositionPotence); //lire la prom
              Serial.println("j'ai lu");
              Serial.println(bonnePositionPotence);
              potence.setObjectif(bonnePositionPotence);
              etatCalibrateur=MESREFPULSE;
						}
				 
					if (boutonP.hasBeenDoubleClicked())
					{
						boutonP.acquit();
						
						Serial.println(F("tourner potar à fond à gauche"));
						etatCalibrateur=REGPOTENCINVIT;
					} 
					break;

          
          case REGPOTENCINVIT :
					
					if (lePotar.getValue()>1000)
					{
						etatCalibrateur=REGLPOTENC;
						Serial.println(F("quand potence reglé (sur le disque optique) faire 1 clic"));
					}   
					break;   

				case REGLPOTENC :
					// Serial.print("obj potence=");
					// Serial.print(potence.getObjectif());
					// Serial.print("   Encours potence=");
					// Serial.print(potence.getEnCours());
					// Serial.print("   obj Pot=");
					// Serial.println(map(lePotar.getValue(),0,1023,potence.getMin(),potence.getMax()));
					// Serial.println("_____");
					potence.setObjectif(map(lePotar.getValue(),0,1023,potence.getMin(),potence.getMax()));
					if (boutonP.hasBeenClicked())
						{
							bonnePositionPotence=map(lePotar.getValue(),0,1023,potence.getMin(),potence.getMax());
							boutonP.acquit();
              EEPROM.put(ADRESSE_EEPROM, bonnePositionPotence);//enregistre en eeprom
              Serial.println("sauver en prom");
							etatCalibrateur=MESREFPULSE;
						}
					break;
			 
				case MESREFPULSE: //mesure des impulsion de reference sur une rotation de delta faite X fois pour plus de certitude
       
          compteurRef=0;
          compteur=0;
          for (i=0;i<NBRCYCLE_AR;i=i+1)
          {
    					Serial.print(F("iteration de mesure de ref="));
    					
    					
    					leServo.setObjectif(leServo.getMilieu()+DELTA);
    					sleep(TEMPOSLEEP);
              mesureRef[i]=compteur;
              leServo.setObjectif(leServo.getMilieu());
              sleep(TEMPOSLEEP);
              compteur=0;
              Serial.println(mesureRef[i]);
          }
          


          
					etatCalibrateur=FINDMINMAX;
					compteurRef=moyenne(NBRCYCLE_AR,mesureRef);
          Serial.print(" SUPERcmpteurRefMoyenne=");
          Serial.println(compteurRef);



          
					if (compteurRef==0)
						{
							Serial.println(F("PB : compteur Ref=0 aucune impulsion fourche optique détecté"));
							Serial.println(F("retour au début de procédure"));
							etatCalibrateur=USB;
						}
          if (ERREURCOMPTAGE*3>compteurRef)
             {
              Serial.println("ce serait bien que la marge d'erreur soit de l'ordre de 30% de la mesure de reference");
							Serial.println("je vais faire autre chose.... Programme a recompiler");
              Serial.println("modifier macro ERREURCOMPTAGE et DELTA");
							etatCalibrateur=POTAR;
              affichage.affiche(POTAR);
              }
					break;
				case FINDMINMAX: 
					//Serial.println("lalalal");
					//sequence pour aller au max puis au min et de déterminer les pwm Min et Max du servo
					compteur=compteurRef;
					while (compteur>compteurRef-ERREURCOMPTAGE && compteur<compteurRef+ERREURCOMPTAGE)
						{
							Serial.print("M+.....");
//            Serial.print(" ObjTest=");
//            Serial.println(objTest);
                                                        
							compteur=0;
							//objTest=objTest+DELTA;
							leServo.setObjectif(leServo.getObjectif()+DELTA);
							sleep(TEMPOSLEEP);
//            Serial.print(" Objectif =");
//            Serial.println(leServo.getObjectif());
                                                   
              Serial.print(" cmptr=");
              Serial.println(compteur);
					

						}
					leServo.setMax(leServo.getObjectif()-DELTA);
					leServo.setObjectif(leServo.getMilieu());
          Serial.print("MAX=");
          Serial.println(leServo.getMax());
          Serial.println("retourneMilieu");
					sleep(TEMPOSLEEP);
					compteur=compteurRef;
					while (compteur>compteurRef-ERREURCOMPTAGE && compteur<compteurRef+ERREURCOMPTAGE  )
						{
							Serial.print(F("M-......"));
							compteur=0;
							//objTest=objTest-DELTA;
							leServo.setObjectif(leServo.getObjectif()-DELTA);
							sleep(TEMPOSLEEP);
              //Serial.print(" Objectif =");
              //Serial.print(leServo.getObjectif());
                                                        
              Serial.print(" cmpteur=");
              Serial.println(compteur);
						}
					leServo.setMin(leServo.getObjectif()+DELTA);
					leServo.setObjectif(leServo.getObjectif()+DELTA);
          Serial.print("MIN=");
          Serial.println(leServo.getMin());
					sleep(TEMPOSLEEP);
					Serial.println("la le servo est au min et va faire grande course");

					//séquence pour mesurer une vitesse et amplitude moyenne
					//(10 cycle d'aller et retour et enregistrement systématique des mesures)
					
					for (i=0;i<NBRCYCLE_AR*2;i=i+2)
						{
							compteur=0;
							compteurRef=0;//compteur ref utilisé différement dans la suite par rapport à ce qui était fait jusque là
							tempsMesureVitesse=millis();
              Serial.print("tempsMesureVitesse=");
              Serial.println(tempsMesureVitesse);
							leServo.setObjectif(leServo.getMax());
							sleep(TEMPO_STAT);
              Serial.print("cmpt=");
              Serial.println(compteur);
								while (compteur>compteurRef)
								{
									compteurRef=compteur;
									sleep(TEMPO_STAT);
                  Serial.println("yop");
								}
							resultatImpulsion[i]=compteur;
							resultatTemps[i]=millis()-tempsMesureVitesse-TEMPO_STAT;
              Serial.print(F("Cycle "));
              Serial.print(i);
              Serial.print(F(" imp= "));
              Serial.print(resultatImpulsion[i]);
              Serial.print(F(" temps= "));
              Serial.println(resultatTemps[i]);
                                                        
                                                        
							compteur=0;
							compteurRef=0;
							tempsMesureVitesse=millis();
							leServo.setObjectif(leServo.getMin());
							sleep(TEMPO_STAT);
								while (compteur>compteurRef)
									{
									compteurRef=compteur;
									sleep(TEMPO_STAT);
									}
							resultatImpulsion[i+1]=compteur;
							resultatTemps[i+1]=millis()-tempsMesureVitesse-TEMPO_STAT;
              Serial.print(F("Cycle "));
              Serial.print(i+1);
              Serial.print(F(" imp= "));
              Serial.print(resultatImpulsion[i+1]);
              Serial.print(F(" temps= "));
              Serial.println(resultatTemps[i+1]);
                                                        
						}
					Serial.println(F("ATTENTION regarder  et noter le sens de rotation du premier mouvement "));
					Serial.println(F("si pret faire un clic "));
					etatCalibrateur=WAITCLIC;
				  
				  
				case WAITCLIC:
					if (boutonP.hasBeenClicked())
						{
							boutonP.acquit();
							etatCalibrateur=DISPRESULT;
							leServo.setObjectif(leServo.getMax());
						} 
					break;

				case DISPRESULT: 
					
                                        float moyenneImpulsion;
                                        moyenneImpulsion=0;
                                        float moyenneTemps;
                                        moyenneTemps=0;
                                        float EcTypeImpulsion;
                                        float EcTypeTemps;
                                        EcTypeImpulsion=0;
                                        EcTypeTemps=0;

                                        byte j;
	                              				for (j=0;j<NBRCYCLE_AR*2;j++)
                                                {
                                                 moyenneImpulsion=moyenneImpulsion+resultatImpulsion[j];
                                                 moyenneTemps=moyenneTemps+resultatTemps[j];
                                                 }
                                       moyenneImpulsion=moyenneImpulsion/(NBRCYCLE_AR*2);
                                       moyenneTemps=moyenneTemps/(NBRCYCLE_AR*2); 
                                       for (j=0;j<NBRCYCLE_AR*2;j++)
                                                {
                                                 EcTypeImpulsion=EcTypeImpulsion+pow(resultatImpulsion[j]-moyenneImpulsion,2);
                                                 EcTypeTemps=EcTypeTemps+pow(resultatTemps[j]-moyenneTemps,2);
                                                 }
                                      EcTypeImpulsion=sqrt(EcTypeImpulsion);
                                      EcTypeTemps=sqrt(EcTypeTemps);          
                              
                              
                                       
                                       Serial.println(F("Alors quelle sens de rotation ?") ); 
				       Serial.print("En moyenne il y a " );
                                       Serial.print(moyenneImpulsion);
				       Serial.print(F(" impulsions en "));
				       Serial.print(moyenneTemps);
				       Serial.println(F(" ms"));          
                                      
				       Serial.print(F(" Ecart Type impulsion " ));
                                       Serial.print(EcTypeImpulsion);
				       Serial.print(F(" Ecart Type temps "));
				       Serial.print(EcTypeTemps);
				       Serial.println(F(" ms")); 
					// a faire calculer moyenne des 2 tableau et calculer ecart type
					//
					///
					potence.setObjectif(potence.getMax());
					Serial.println(F("si nouveau servo : doubleClic, si changement mode clic"));
					etatCalibrateur=WAITCLIC2;               
					break;
				   
				case WAITCLIC2: 
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
							etatCalibrateur=13;
							leServo.setObjectif(leServo.getMilieu());
							potence.setObjectif(bonnePositionPotence);
							sleep(TEMPOSLEEP*3);
						}    
					break ;    
					  
				case SWEEPTOMIN:
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
		
				case SWEPPTOMAX:
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

				case POTAR:
					int temp;
					int potarVal;
					potarVal=lePotar.getValue();
					//Serial.print("lepotar : ");
					//Serial.println(potarVal);
					temp=map(potarVal,0,1023,leServo.getMin(),leServo.getMax());
					// if (leServo.getType())
					// {
					// temp=map(potarVal,0,1023,300,3000);
					// }
					// else
					// {
					// temp=map(potarVal,0,1023,100,1000);
					// }
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
		
				case MILIEU:
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
	affichage.affiche(POTAR);
	attachInterrupt(0, updatePulseCompteur, CHANGE ); // Int0= la pin N°2 sur un UNO
}
 void updatePulseCompteur()
{ 
	compteur++;       
}     



         
void loop ()
{
	mySCoop.sleep(1);
	if (millis()>2000+temp)
		{
			temp=millis();
		}

}

