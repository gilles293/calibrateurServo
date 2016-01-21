/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
# 
# Program principal:

// A FAIRE : gérer les vitesse : les mettre à fond quand test usb les mettre plus bas pour les autres modes.
// A FAIRE : au démarage de l'arduino initialiser les valeurs potence loin de disque optique.

#*******************************************************************************
 J.Soranzo
	16/12/2015 gestion sous git + github
	https://github.com/gilles293/calibrateurServo.git
	nettoyage du code, j'ai craqué
*/ 



#include <SCoop.h>
#include <Wire.h>
#include "bouton.h"
#include "afficheur.h"
#include "servoTest.h"
#include "potar.h"
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>
#define DELTA 150
#define TEMPOSLEEP 1000 //en ms le temps pour le servo de bouger d'un increment
#define ERREURCOMPTAGE 3 //la tolérance de comptage avant que l'on estime que le servo est arrivé au bout


bouton boutonP(4);
afficheur affichage(0x20);
int bonnePositionPotence;
byte etatCalibrateur (3); //1= PC 2=Sweep 3=potar 4=Milieu Par défaut = 3 au démarage
unsigned long temp;
unsigned long tempsMesureVitesse;
potar lePotar(0);
servoTest leServo(5); 
servoTest potence(11);
unsigned long dateChangeServo;
bool changeTypeServo(false);
int compteur(0);
int compteurRef(0);
int resultatImpulsion[19];
long resultatTemps[19];

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
		SWEEPTOMIN = 2,
		SWEPPTOMAX = 20
};


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

		int objTest (0);
		switch (etatCalibrateur)
			{
				case USB :
					Serial.println(F("Double Clic pour procéder à l'etlonnage"));
					etatCalibrateur=ETALORSWEEP;
					break;

				case ETALORSWEEP :
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
						leServo.setVitesse(4000);
						Serial.println(F("tourner potar à fond à gauche"));
						etatCalibrateur=REGPOTENCINVIT;
					} 
					break;   

				case REGPOTENCINVIT :
					leServo.setObjectif(leServo.getMilieu());
					// pour etre certain qu'on aprt du milieu et que le servo 
					//n'est pas n'importe ou suite à des manip de type sweep
					//ou potar par ex
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
							etatCalibrateur=MESREFPULSE;
						}
					break;
			 
				case MESREFPULSE: //mesure des impulsion de reference sur une rotation de delta
					Serial.println("ICI");
					compteur=0;
					compteurRef=0;
                                        Serial.print("En Cours=");
                                        Serial.print(leServo.getEnCours());
                                        Serial.print(" Objectif =");
                                        Serial.println(leServo.getObjectif());
					objTest=leServo.getMilieu()+DELTA;
					leServo.setObjectif(objTest);
                                        Serial.println("Envoi incrément Servo");
                                        Serial.println("En Cours=");
                                        Serial.print(leServo.getEnCours());
                                        Serial.print(" Objectif =");
                                        Serial.println(leServo.getObjectif());
					sleep(TEMPOSLEEP);
                                        Serial.println("Fin attente");
                                        Serial.println("En Cours=");
                                        Serial.print(leServo.getEnCours());
                                        Serial.print(" Objectif =");
                                        Serial.println(leServo.getObjectif());
					etatCalibrateur=FINDMINMAX;
					Serial.println("la");
					compteurRef=compteur;
					if (compteurRef==0)
						{
							Serial.println(F("PB : compteur Ref=0 aucune impulsion fourche optique détecté"));
							Serial.println(F("retour au début de procédure"));
							etatCalibrateur=1;
						}
					break;
				case FINDMINMAX: 
					Serial.println("lalalal");
					//sequence pour aller au max puis au min et de déterminer les pwm Min et Max du servo
					while (compteur>compteurRef-ERREURCOMPTAGE)
						{
							Serial.println("LAICI");
							compteur=0;
							objTest=objTest+DELTA;
							leServo.setObjectif(objTest);
							sleep(TEMPOSLEEP);
						}
					leServo.setMax(objTest-DELTA);
					leServo.setObjectif(leServo.getMilieu());
					sleep(TEMPOSLEEP);
					compteur=compteurRef;
					while (compteur>compteurRef-ERREURCOMPTAGE)
						{
							Serial.println(F("LAICIlabas"));
							compteur=0;
							objTest=objTest-DELTA;
							leServo.setObjectif(objTest);
							sleep(TEMPOSLEEP);
						}
					leServo.setMin(objTest+DELTA);
					leServo.setObjectif(objTest+DELTA);
					sleep(TEMPOSLEEP);
					
					//séquence pour mesurer une vitesse et amplitude moyenne
					//(10 cycle d'aller et retour et enregistrement systématique des mesures)
					byte i;
					for (i=0;i<20;i++)
						{
							compteur=0;
							compteurRef=0;//compteur ref utilisé différement dans la suite
							tempsMesureVitesse=millis();
							leServo.setObjectif(leServo.getMax());
							sleep(200);
								while (compteur>compteurRef);
								{
									compteurRef=compteur;
									sleep(200);
								}
							resultatImpulsion[i]=compteur;
							resultatTemps[i]=millis()-tempsMesureVitesse-200;
							compteur=0;
							compteurRef=0;
							tempsMesureVitesse=millis();
							leServo.setObjectif(leServo.getMin());
							sleep(200);
								while (compteur>compteurRef);
									{
									compteurRef=compteur;
									sleep(200);
									}
							resultatImpulsion[i+1]=compteur;
							resultatTemps[i+1]=millis()-tempsMesureVitesse-200;
						}
					Serial.println(F("ATTENTION regarder  et noter le sens de rotation du premier mouvement "));
					Serial.println(F("si pret faire un clic "));
					etatCalibrateur=15;
				  
				  
				case WAITCLIC:
					if (boutonP.hasBeenClicked())
						{
							boutonP.acquit();
							etatCalibrateur=16;
							leServo.setObjectif(leServo.getMax());
						} 
					break;

				case DISPRESULT: 
					Serial.println(F("Alors quelle sens de rotation ?") ); 
					Serial.print(resultatImpulsion[5]);
					Serial.print(F(" impulsions en "));
					Serial.println(resultatTemps[5]);
					Serial.println(F(" ms")); 
					//
					//
					//
					//
					// a faire calculer moyenne des 2 tableau et calculer ecart type
					//
					///
					potence.setObjectif(potence.getMax());
					Serial.println(F("si nouveau servo : doubleClic, si changement mode clic"));
					etatCalibrateur=17;               
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
	if (millis()>2000+temp)
		{
			temp=millis();
		}

}

