/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gilles De Bussy
# 
# Program principal:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

si probleme à la compilation "defined in discarded section" trouver wiring.c (sur 2 PC différent se 
trouve à 2 endroit différend. Le pister
 grace au wiring.c.d (dont on connait la position en mettant l'interface arduino en mode bavard au moment de la compilation)
 qui est fabriqué au moment de la compil)
et ajouter l'attribut :

__attribute__((used)) volatile unsigned long timer0_overflow_count = 0;


RAF verif affichage LCD, verif fonctionnement avec Adafruit, ecrire Mode d'emploi
poursuivre modif code pour delta dynamique en fonction de ADAFRUIT ou CLASSIQUE

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!




//A Faire : vérifier l'utilité voir les errreur commise par tempo sleep avant les yop et les yup


#*******************************************************************************
 J.Soranzo
    16/12/2015 gestion sous git + github
    https://github.com/gilles293/calibrateurServo.git
    nettoyage du code, j'ai craqué
*/ 
//------------------------------------------------------------------------


#include <EEPROM.h>
#include <SCoop.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Servo.h>

//------------------------------------------------------------------------------
//Lib projet
#include "bouton.h"
#include "afficheur.h"
#include "servoTest.h"
#include "potar.h"

//------------------------------------------------------------------------------
#define DELTACLASSIQUE 150 // en servo unit (en microseconde de servo)
#define DELTAADAFRUIT 30 // en quantum sur 4096 bit
#define TEMPOSLEEP 1000
//en ms le temps d'etre sur que le servo ait bougé 
//d'un increment quand on fait les mesure à la roue codeuse
#define PINVOIEB 3 //la pin sur laquelle est branché la voie B de la roue codeuse
#define ERREURCOMPTAGE 6 //la tolérance de comptage avant que l'on estime que le servo est arrivé au bout
#define NBRCYCLE_AR 4 //nbre de cycle d'aller et retour pour la fin des mesure de caractérisation et nbr de cycle pour la mesure des PWMIN et PWMMAX
#define TEMPO_STAT 40 // tempo pour laisser le servo avancer lors des mesures
                       // statistique à la fin de la calibration ATTENTION 100ms semble etre trop court!!!!
#define DEP_MINI_STAT 5  //correspond au déplacement minimum en nombre d'impulsion que doit faire le servo pour 
						//considérer qu'il n' pas encore atteint sa fin de course et que l'on peut laisser le chronometre en 
						//route (pour mesure de vitesse réeel du servo en °/s)
#define ADRESSE_EEPROM 42//position de leeprom ou la valeur de reglage de vitesse est stocké
#define VITESSEMAXSERVO 4000 // vitesse max d'un servo en microsecond de PWM par seconde
//cette vitesse doit imperativement etre superieure à la vitesse que
//peut physiquement atteindre le servo afin que les mesrues de vitesse physique en fin de calibration USB soient justes.
//Par conttre dans la sequense sweep to min sweep to max cette valeur conduit a 
//empecher le servo d'atteindre le min et le max,
//il faut reduire la vitesse avec le potar pour que le servo
//ait le temps d'atteindre physiquement ses postion min et max
#define PINSERVO 5 // pin arduino ou est branché le servo 

//------------------------------------------------------------------------------
// Etats de la machine d'état
enum {
        POTAR = 3,
        MILIEU = 4,
        USB = 1,
        ETALORSWEEP = 10,
        //REGPOTENCINVIT = 11,
        REGLPOTENC = 12,
        MESREFPULSE = 13,
        FINDMINMAX = 14,
        //WAITCLIC = 15,
        DISPRESULT = 16,
        WAITCLIC2 = 17,
        //LECTURE_OU_ECRITURE_EEEPROM=18,
        SWEEPTOMIN = 2,
        SWEEPTOMAX = 20,
        VITESSE=22,
};

//------------------------------------------------------------------------------
//Variables globales
bouton boutonP(4);
afficheur affichage(0x20);
//int bonnePositionPotence;
byte etatCalibrateur (POTAR); //1= PC 2=Sweep 3=potar 4=Milieu Par défaut = 3 au démarage
unsigned long temp;
unsigned long tempsMesureVitesse;
potar lePotar(0);
servoTest leServo(PINSERVO); 
//servoTest potence(11);
unsigned long dateChangeServo;
bool changeTypeServo(false);
int delta(0);
int compteur(0); //variable compteur d'impulsions
                //mise à jour par interruptions
                
int compteurRef(0);
int amplitude(0);
//int resultatAmplitudeMax[NBRCYCLE_AR]; 
long resultatTempsMax[NBRCYCLE_AR];
int resultatMaxServo[NBRCYCLE_AR]; // en ServoUnit (us)
int resultatAmplitude[NBRCYCLE_AR]; //en unité roue codeuse (quasi des degré)
long resultatTempsMin[NBRCYCLE_AR];
int resultatMinServo[NBRCYCLE_AR]; // en ServoUnit (us)

int mesureRef[NBRCYCLE_AR];
int PWMMin[NBRCYCLE_AR];
int PWMMax[NBRCYCLE_AR];
int vitesseTemp;


int objTest (0); // varaible temporaire pour ...peut sans doute etre supprimé
// en faisant apelle aux bnnes méthode pour connaitre etat courrant du servo

//------------------------------------------------------------------------------
//fonctions de calculs 
long ecartType (int tailleTab, int tab [])
{
    long moy(moyenne(tailleTab,tab));
    long ecType(0);
    int j(0);
    for (j=0;j<tailleTab;j++)
    {
        ecType=ecType+pow(tab[j]-moy,2);  
    }
    ecType=sqrt(ecType/tailleTab);
    return ecType;          
}                                     

long ecartType (int tailleTab, long tab [])
{
    long moy(moyenne(tailleTab,tab));
    long ecType(0);
    int j(0);
    for (j=0;j<tailleTab;j++)
    {
        ecType=ecType+pow(tab[j]-moy,2);
    }
    ecType=sqrt(ecType/tailleTab);
    return ecType;          
}                

long moyenne (int tailleTab, long tab [])
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

long moyenne (int tailleTab, int tab [])
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

//------------------------------------------------------------------------------
//Scoop Timers
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

defineTimerRun(surveilleServo,100) //conserver 100 ou modifier le "*0.1"
//dans méthode appliqueobjectif du Servo
{  
     //Serial.println("Servo");
     //Serial.println("potence.
     leServo.appliqueObjectif();
     //potence.appliqueObjectif();
}

defineTimerRun(surveillePotar,80)
{
   // Serial.println("potarrefresh");
    lePotar.refresh();
}


//------------------------------------------------------------------------------
//La tache Scoop
defineTask(reflechi,250)
    void reflechi::setup(){ }
    void reflechi::loop()
    //defineTimerRun(reflechi,40)
/*
  AFFICHERIEN,
  AFFICHEUSB,
  AFFICHESWEEP,
  AFFICHEPOTAR,
  AFFICHEMILIEU,
  AFFICHEADAFRUIT,
  AFFICHECLASSIQUE
  */
  
    {
		
    byte i;
        //Test de changement de servo moteur ADAFRUIT ou NORMAL
        if (boutonP.hasBeenLongClicked())
            { 
				
                dateChangeServo=(millis());
                changeTypeServo=true;
				
                if (leServo.getType())
                    {
                        leServo.setType(false);
						delta=DELTAADAFRUIT;
                        affichage.affiche(AFFICHEADAFRUIT);
                        Serial.println(F("servo de type Adafruit"));
                    }
                else
					
                    {
                        leServo.setType(true);
                        affichage.affiche(AFFICHECLASSIQUE);
                        Serial.println(F("servo de type Classique"));
						delta=DELTACLASSIQUE;
					
                    }
					
                //sleep( 3000);
                boutonP.acquit();
				
            }
			
		
        //Affichage du nouveau type de servo moteur pendant 3s
        if (millis()>dateChangeServo+3000 && changeTypeServo)
            {
                affichage.affiche(etatCalibrateur);
                changeTypeServo=false;
            }

        
        switch (etatCalibrateur)
            {
                case USB :
                    Serial.println(F(\
          "Double Clic pour proceder a mesure via USB, sinon simple clic change de mode"));                              
                    etatCalibrateur=ETALORSWEEP;
                    break;

                case ETALORSWEEP :
                    if (boutonP.hasBeenClicked())
                        {
                            boutonP.acquit();
                            etatCalibrateur=SWEEPTOMIN;
                            affichage.affiche(SWEEPTOMIN);
                            leServo.setObjectif(leServo.getMin());
                            lePotar.acquit();
                        }
                 
                    if (boutonP.hasBeenDoubleClicked())
                    {
                        boutonP.acquit();
                        leServo.setVitesse(VITESSEMAXSERVO);
						Serial.print("vitesse=");
						Serial.println(VITESSEMAXSERVO);
						Serial.print("tempo stat=");
						Serial.println(TEMPO_STAT);
                        leServo.setObjectif(leServo.getMilieu());
         // pour etre certain qu'on part du milieu et que le servo 
          //n'est pas n'importe ou suite à des manip de type sweep
          //ou potar par ex
                        //etatCalibrateur=LECTURE_OU_ECRITURE_EEEPROM;
                        etatCalibrateur=MESREFPULSE;
                    } 
                    break;   

                case MESREFPULSE:
                //mesure des impulsions  de reference en provenance du capteur optique sur une rotation
                //de delta faite X fois pour plus de certitude
                    compteurRef=0;
                    compteur=0;
                    for (i=0;i<NBRCYCLE_AR;i=i+1)
                    {
                        //mesure milieu+delta, tempo, get_compteur
						//milieu, tempo, raz_compteur
						Serial.print(F("iteration de mesure de ref="));
                        leServo.setObjectif(leServo.getMilieu()+delta);
                        sleep(TEMPOSLEEP);
                        mesureRef[i]=compteur;
                        leServo.setObjectif(leServo.getMilieu());
                        sleep(TEMPOSLEEP);
                        compteur=0;
                        Serial.println(mesureRef[i]);
                    }
                    etatCalibrateur=FINDMINMAX;
					//Affichage de ce resultat
                    compteurRef=moyenne(NBRCYCLE_AR,mesureRef);
                    Serial.print(F(" SUPERcmpteurRefMoyenne vue à la fourche optique = "));
                    Serial.print(compteurRef);
                    Serial.print(F(" (ecart type : "));
                    Serial.println(ecartType(NBRCYCLE_AR,mesureRef));
					Serial.println(F(" ) "));
					Serial.println();
					// 2 petites vérification ref <> 0 et ref pqs trop petit
                    if (compteurRef==0)
                        {
                            Serial.println(F("PB : compteur Ref=0 aucune impulsion fourche optique détecté"));
                            Serial.println(F("retour au début de procédure"));
                            etatCalibrateur=USB;
                        }
                    if (ERREURCOMPTAGE*3>abs(compteurRef))
                    {
                        Serial.println(F("ce serait bien que la marge d'erreur soit de l'ordre de 30% de la mesure de reference"));
                        Serial.println(F("je vais faire autre chose.... Programme a recompiler"));
                        Serial.println(F("modifier macro ERREURCOMPTAGE et DELTAADAFRUIT ou DELTACLASSIQUE"));
                        etatCalibrateur=POTAR;
                        affichage.affiche(POTAR);
                    }
                    break;
                case FINDMINMAX: 
                    
                    //sequence pour aller au max puis au min et de déterminer
                    //les pwm Min et Max du servo
                    //----------------------------------------------------------
                    //find max
					// Principe : on commande un delta déplacement, on laisse
					//   compter et on regarde si le compteur du capteur optique est égal
					//   à ref +/- une petite erreur autorisée
					//   Si oui alors on recommence sinon on doit etre en butee
                    for (i=0;i<NBRCYCLE_AR;i=i+1){
                        compteur=compteurRef;
                        amplitude=0;
						Serial.print(F("Cycle recherche MAX n ") );Serial.println(1+i);
                        while (abs(compteurRef-compteur)<ERREURCOMPTAGE) {
                            
                            Serial.print(F("M+....."));
                            compteur=0;
                    
                            leServo.setObjectif(leServo.getObjectif()+delta);
                            sleep(TEMPOSLEEP);
							if (leServo.getType())
							{
							Serial.print(F(" PWM =")); Serial.print(leServo.getObjectif());
                            Serial.print(F("usec,  cmptr fourche optique=")); Serial.print(compteur);	
							}
							else
							{
							Serial.print(F(" quantum sur 4096 =")); Serial.print(leServo.getObjectif());
                            Serial.print(F("usec,  cmptr fourche optique=")); Serial.print(compteur);	
							}
							
                            amplitude=amplitude+abs(compteur);
                            Serial.print(F(" amplitude fourche optique="));Serial.println(amplitude);
                        }
                        amplitude=amplitude-abs(compteur)	;
                    
                        resultatMaxServo[i]= leServo.getObjectif()-delta;
                        Serial.print(F(" Amplitude max fourche optique par rapport au milieu retenue =") );Serial.println(amplitude);
						if (leServo.getType())
							{
							Serial.print(F(" PWM Max retenue =")); Serial.print(resultatMaxServo[i]);
                            Serial.println(F("usec"))	
							}
							else
							{
							Serial.println(F(" quantum Max sur 4096 retenue=")); Serial.print(resultatMaxServo[i]);
                            	
							}
                    
					leServo.setObjectif(leServo.getMilieu());
                    Serial.println("retourneMilieu");
                    sleep(TEMPOSLEEP);
                    //----------------------------------------------------------
                    
                        compteur=-compteurRef;
						Serial.print(F("Cycle recherche MIN n ") );Serial.println(i+1);
                        while (abs(-compteurRef-compteur)<ERREURCOMPTAGE){
                        // inversion du signe de compteur ref car on change de sens
                        // de rotation (par rapport au sens de recherche de compteur ref)
                                                   
                            Serial.print(F("M-......"));
                            compteur=0;
                    
                            leServo.setObjectif(leServo.getObjectif()-delta);
                            sleep(TEMPOSLEEP); 
                            if (leServo.getType())
							{
							Serial.print(F(" PWM =")); Serial.print(leServo.getObjectif());
                            Serial.print(F("usec,  cmptr fourche optique=")); Serial.print(compteur);	
							}
							else
							{
							Serial.print(F(" quantum sur 4096 =")); Serial.print(leServo.getObjectif());
                            Serial.print(F("usec,  cmptr fourche optique=")); Serial.print(compteur);	
							}
                            amplitude=amplitude+abs(compteur);
                            Serial.print(F(" amplitude="));Serial.println(amplitude);
                        }
                        amplitude=amplitude-abs(compteur)	;
                        resultatAmplitude[i] = amplitude ;
                        resultatMinServo[i]= leServo.getObjectif()+delta;
						leServo.setObjectif(leServo.getObjectif()+delta);
                        
						
						Serial.print(F(" Amplitude min fourche optique par rapport au milieu retenue =") );Serial.println(amplitude);
						if (leServo.getType())
							{
							Serial.print(F(" PWM Min retenue =")); Serial.print(resultatMaxServo[i]);
                            Serial.println(F("usec"))	
							}
							else
							{
							Serial.println(F(" quantum Min sur 4096 retenue=")); Serial.print(resultatMaxServo[i]);
                            	
							}
						
						
						leServo.setObjectif(leServo.getMilieu());
                    
						Serial.println("retourneMilieu");
						sleep(TEMPOSLEEP);
						Serial.println(F("*************************************") );
                    }

                    Serial.println(F("=============") );
					leServo.setMax( (int) moyenne( NBRCYCLE_AR, resultatMaxServo ) );
                    
					if (leServo.getType())
							{
							Serial.print("PWM MAX retenue vue l'ensemble des cycles ="); Serial.print(leServo.getMax());
							Serial.print(F(" usec d'angle   (ecart type : "));
							}
							else
							{
							Serial.print("Quantum MAX retenue vue l'ensemble des cycles ="); Serial.print(leServo.getMax());
							Serial.print(F(" sur 4096   (ecart type : "));	
							}
					
					
					
					
                    Serial.println(ecartType(NBRCYCLE_AR,resultatMaxServo));
					Serial.print(F(")"));
					leServo.setMin((int) moyenne( NBRCYCLE_AR, resultatMinServo ));
					
					
					if (leServo.getType())
							{
							Serial.print("PWM Min retenue vue l'ensemble des cycles ="); Serial.print(leServo.getMin());
							Serial.print(F(" usec d'angle   (ecart type : "));
							}
							else
							{
							Serial.print("Quantum Min retenue vue l'ensemble des cycles ="); Serial.print(leServo.getMin());
							Serial.print(F(" sur 4096   (ecart type : "));	
							}
					
					
					
					
                   			
					
                    Serial.println(ecartType(NBRCYCLE_AR,resultatMinServo));
                    Serial.print(F(")"));
                    
                    
                    //----------------------------------------------------------

                    //Amplitude angulaire exprimee en unite de roue codeuse
					//amplitude = moyenne( NBRCYCLE_AR, resultatAmplitudeMax );
                    amplitude = moyenne( NBRCYCLE_AR, resultatAmplitude );
					
					Serial.print(F("amplitude moyenne retenue vue par la fourche optique = ") );Serial.print(amplitude);
					Serial.print(F(" impulsions (ecart type : "));
                    Serial.println(ecartType(NBRCYCLE_AR,resultatAmplitude));
					Serial.print(F(" )"));
					
                    Serial.println(F("=============") );
					
                    leServo.setObjectif(leServo.getMin());
					sleep(TEMPOSLEEP);
					Serial.println("en ce moment le servo est au min et va faire grande course plusieurs fois de suite pour déterminer la vitesse du servo");
					//----------------------------------------------------------
                    //séquence pour mesurer une vitesse et amplitude moyenne

                    //(10 cycles d'aller et retour de min à max
                    //et enregistrement systématique des mesures)
					//chaque cycle s'arrete quand le servo atteint amplitude moyenne à DEP_MIN_STAT près
                    for (i=0;i<NBRCYCLE_AR;i=i+1)
                        {
                            compteur=0;
                            tempsMesureVitesse=millis();
                            //Serial.print("tempsMesureVitesse=");
                            //Serial.println(tempsMesureVitesse);
                            // Mesure de min vers max
                            leServo.setObjectif(leServo.getMax());
                            //sleep(TEMPOSLEEP); inutile maintenant car on s'arrette quand compteur suffisament incrémenté'
                            //Serial.print("cmpt="); Serial.println(compteur);
                            while (abs(compteur)<abs(amplitude)-DEP_MINI_STAT)
                                {
                                   /*  Serial.print("yop");
                                    Serial.print(" cmpt=");
                                    Serial.println(compteur); */
                                    sleep(TEMPO_STAT);
                                }
                            
                            resultatTempsMax[i]=millis()-tempsMesureVitesse;//-\
                                                    TEMPO_STAT;
                            Serial.print(F("Cycle ")); Serial.print(i);
                            Serial.print(F(" vers le max, temps= ")); Serial.print(resultatTempsMax[i]);
							Serial.println(F(" ms "));
                        
                            compteur=0;
                            
                            tempsMesureVitesse=millis();
                            // Mesure de max vers min
                             //Serial.print("tempsMesureVitesse=");
                            //Serial.println(tempsMesureVitesse);
                            leServo.setObjectif(leServo.getMin());
                            //sleep(TEMPOSLEEP);
                            Serial.print("cmpt="); Serial.println(compteur);
                            while (abs(compteur)<abs(amplitude)-DEP_MINI_STAT)
                                {
                                    
                                    /*Serial.print("yup ");
                                    Serial.print("cmpt=");
                                    Serial.println(compteur);*/
                                    sleep(TEMPO_STAT);
                                }

                            
							
                            resultatTempsMin[i]=millis()-tempsMesureVitesse;//-TEMPO_STAT;
                            Serial.print(F("Cycle ")); Serial.print(i);
                            Serial.print(F(" vers le max, temps= ")); Serial.print(resultatTempsMin[i]);
							Serial.println(F(" ms "));
							
							Serial.println(F("********************************** "));
                            
							
                        } //fin for NBCYCLES
   
         
					sleep(TEMPO_STAT);
					compteur=0;
                    leServo.setObjectif(leServo.getMax());
					etatCalibrateur=DISPRESULT;
					Serial.println();
                    Serial.println();
					if (compteur<0)
						{
							Serial.println(F("Le servo tourne dans le sens direct "));
							
						}
					else
					{
					Serial.println(F("Le servo tourne dans le sens indirect "));	
					}
					
                  
                

                case DISPRESULT:  
                    
                    leServo.setObjectif(leServo.getMilieu());
					if (leServo.getType())
					{
					Serial.println(F(" Servo CLASSIQUE"));	
					}
					else
					{
						Serial.println(F(" Servo ADAFRUIT"));
					}
					
                    
					
					
					Serial.print(F(" Dans le sens croissant, "));
                   
                    Serial.print(F("le max est atteint en un temps moyen de "));
                    Serial.print(moyenne(NBRCYCLE_AR,resultatTempsMax));
                    Serial.print(F(" ms (ecart type : "));
                    Serial.print(ecartType(NBRCYCLE_AR,resultatTempsMax));
                    Serial.println(F(" ms)"));
                    Serial.print(F(" Dans le sens decroissant , "));
                    //Serial.print(moyenne(NBRCYCLE_AR,resultatAmplitude));
                    //Serial.print(F("impulsions (Ecart type : "));
                    //Serial.print(ecartType(NBRCYCLE_AR,resultatAmplitude));
                    Serial.print(F("le min est atteint en un temps moyen de "));
                    Serial.print(moyenne(NBRCYCLE_AR,resultatTempsMin));
                    Serial.print(F(" ms (ecart type : "));
                    Serial.print(ecartType(NBRCYCLE_AR,resultatTempsMin));
                    Serial.println(F(" ms)"));
                    //potence.setObjectif(potence.getMax());
					Serial.println();
                    Serial.println(F("si nouveau servo : doubleClic, si changement mode clic"));
                    etatCalibrateur=WAITCLIC2;               
                    break;
                   
                case WAITCLIC2:  
                    if (boutonP.hasBeenClicked())
                        {
                            boutonP.acquit();
                            etatCalibrateur=SWEEPTOMIN;
                            affichage.affiche(SWEEPTOMIN);
                            leServo.setObjectif(leServo.getMin());
              Serial.print(F("bouger le potar modifie la vitesse, double click la sauve en prom "));
                        } 
                    if (boutonP.hasBeenDoubleClicked())
                        {
                            boutonP.acquit();
                            etatCalibrateur=MESREFPULSE;
                            leServo.setType(leServo.getType());
							leServo.setVitesse(VITESSEMAXSERVO);
                            leServo.setObjectif(leServo.getMilieu());
                            //potence.setObjectif(bonnePositionPotence);
                            sleep(TEMPOSLEEP*3);
                        }    
                    break ;    
                      
                case SWEEPTOMIN:
                    if (leServo.isMin())
                        {
                            etatCalibrateur=SWEEPTOMAX;
                            leServo.setObjectif(leServo.getMax());
                        }
                    if (boutonP.hasBeenClicked())
                        {
                            boutonP.acquit();
                            lePotar.acquit();
                            etatCalibrateur=POTAR;
                            affichage.affiche(POTAR);
                        }
                    if (lePotar.hasBeenMovedALot())
                        {
                         //Serial.println("plip");
                          leServo.setVitesse(map(lePotar.getValue()\
                                                ,0,1023,0,VITESSEMAXSERVO));
                        }
                    if (boutonP.hasBeenDoubleClicked())
                        {
                            boutonP.acquit();
                            lePotar.getValue();
                            //enregistre en eeprom
                            EEPROM.put(ADRESSE_EEPROM, map(lePotar.getValue(),0,1023,0,VITESSEMAXSERVO));
                            Serial.print(F("sauver en prom vitesse = "));
                            Serial.print(  map(lePotar.getValue(),0,1023,0,VITESSEMAXSERVO));
                            Serial.print(F("microsec de PWM par seconde"));
                        }
           
                    break;
        
                case SWEEPTOMAX:
                    if (leServo.isMax())
                        {
                            etatCalibrateur=SWEEPTOMIN;
                            leServo.setObjectif(leServo.getMin());
                        }
                    if (boutonP.hasBeenClicked())
                        {
                            boutonP.acquit();
                            lePotar.acquit();
                            etatCalibrateur=POTAR;
                            affichage.affiche(POTAR);
                        }
                    if (lePotar.hasBeenMovedALot())
                        {
                            //Serial.println("plup");
                            leServo.setVitesse(map(lePotar.getValue(),\
                                                    0,1023,0,VITESSEMAXSERVO));
                        }
                    if (boutonP.hasBeenDoubleClicked())
                        {
                            boutonP.acquit();
                            lePotar.getValue();
                            //enregistre en eeprom
                            EEPROM.put(ADRESSE_EEPROM,\
                                        map(lePotar.getValue(),0,1023,0,\
                                        VITESSEMAXSERVO));
                            Serial.print(F("sauver en prom vitesse = "));
                            Serial.print( map(lePotar.getValue(),\
                                                0,1023,0,VITESSEMAXSERVO));
                            Serial.print(F("microsec de PWM par seconde"));
                        }
                    break;

                case POTAR:
                    int temp;
                    int potarVal;
                    potarVal=lePotar.getValue();
                    temp=map(potarVal,0,1023,leServo.getMin(),leServo.getMax());   
                    leServo.setObjectif(temp);
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
  int vitesseDinit;    
  pinMode(PINVOIEB, INPUT);      
    Serial.begin(57600);

  lePotar.init();
    leServo.setType(true);
	delta=DELTACLASSIQUE;
  EEPROM.get(ADRESSE_EEPROM, vitesseDinit);

  if (vitesseDinit<=VITESSEMAXSERVO && vitesseDinit>=0)
    {leServo.setVitesse(vitesseDinit);
    Serial.print("vitesse EEPROM=");
	Serial.print(vitesseDinit);
   
    }
   else
   {
    Serial.println("emploi de la vitesse 300");
    leServo.setVitesse(300);
   }
  
    Wire.begin();

    Serial.println("hello");
    
	mySCoop.start();
	
    temp=millis();
    //etatCalibrateur=3; fait dans l'init de la variable
    affichage.affiche(POTAR);
    attachInterrupt(0, updatePulseCompteur, RISING ); // Int0= la pin N°2 sur un UNO
}
 void updatePulseCompteur()
{ 
    if (digitalRead(PINVOIEB))
		{compteur++;       }
	else
		{compteur--;}

}     

int freeRam () {
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

         
void loop ()
{
    	mySCoop.sleep(1);
    /*
	if (millis()>5000+temp)
        {
            temp=millis();
			Serial.println();
            Serial.print("stackLeftScoop=");
            Serial.println(reflechi.stackLeft());
			Serial.print("freeRam=");
            Serial.println(freeRam());
        }
	*/
	
}

