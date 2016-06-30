/**
#*******************************************************************************
# Projet : VOR-005
# Sous projet : calibrateur de servo moteurs
# Auteurs: Gille De Bussy
# 
# Program principal:


//nettoyer code de la potence (2e servo)

//afaire : faire un réinit propre car si on fait 2 étalonnage à la suite ca marche pas
//a Faire : lire l'autre voie de la roue codeuse pour detectr les rebond du servo

//a faire tester repetabilité avec tempo à 100 et à 200.
Avec 100 on a des comportement louche lors des cycles de fin 
(le servo bouge apeine sur certains des cycle a la palce d'un grand débatement)
// a faire : comprendre pourquoi les vitesses sont pas les memes selon les modes.


#*******************************************************************************
J.Soranzo
    16/12/2015 gestion sous git + github
    https://github.com/gilles293/calibrateurServo.git
    nettoyage du code, j'ai craqué
*/ 
//------------------------------------------------------------------------------
//pour la suite faire moyenne pour mesure PWMIN et PWMMAX et virer 
//le calcul de la moyenne mesureimpulsion pour le remplacer par la méthode


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
#define DELTA 150
#define TEMPOSLEEP 800
//en ms le temps pour le servo de bouger d'un increment
//quand on fait les mesure à la roue codeuse

#define ERREURCOMPTAGE 15 //la tolérance de comptage avant que l'on estime que le servo est arrivé au bout
#define NBRCYCLE_AR 10 //nbre de cycle d'aller et retour pour la fin des mesure de caractérisation et nbr de cycle pour la mesure des PWMIN et PWMMAX
#define TEMPO_STAT 100 // tempo pour laisser le servo avancr lors des mesures
// statistique à la fin de la calibration
#define ADRESSE_EEPROM 42//position de leeprom ou la valeur de reglage de vitesse est stocké
#define VITESSEMAXSERVO 3500 // vitesse max d'un servo en microsecond de PWM par seconde
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
    WAITCLIC = 15,
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

int compteur = 0; //variable compteur d'impulsions
//mise à jour par interruptions

int compteurRef = 0;
int resultatImpulsionMax[NBRCYCLE_AR];
long resultatTempsMax[NBRCYCLE_AR];
int resultatImpulsionMin[NBRCYCLE_AR];
long resultatTempsMin[NBRCYCLE_AR];

int mesureRef[NBRCYCLE_AR];
int PWMMin[NBRCYCLE_AR];
int PWMMax[NBRCYCLE_AR];
int vitesseTemp;

//------------------------------------------------------------------------------
//fonctions de calculs 
long ecartType (int tailleTab, int tab []){
    long moy(moyenne(tailleTab,tab));
    long ecType(0);
    int j(0);
    for (j=0;j<tailleTab;j++){
        ecType=ecType+pow(tab[j]-moy,2);  
    }
    ecType=sqrt(ecType);
    return ecType;          
}                                     

long ecartType (int tailleTab, long tab []){
    long moy(moyenne(tailleTab,tab));
    long ecType(0);
    int j(0);
    for (j=0;j<tailleTab;j++){
        ecType=ecType+pow(tab[j]-moy,2);
    }
    ecType=sqrt(ecType);
    return ecType;          
}                

long moyenne (int tailleTab, long tab []){
    byte j;
    long moyenne (0);
    for (j=0;j<tailleTab;j++){
        moyenne=moyenne+tab[j];
    }
    moyenne=moyenne/(tailleTab);
    return moyenne;
}

long moyenne (int tailleTab, int tab []){
    byte j;
    long moyenne (0);
    for (j=0;j<tailleTab;j++){
        moyenne=moyenne+tab[j];
    }
    moyenne=moyenne/(tailleTab);
    return moyenne;  
}
/*
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
*/
/*
//------------------------------------------------------------------------------
//La tache Scoop
defineTask(reflechi)
void reflechi::setup(){ }
void reflechi::loop()
//defineTimerRun(reflechi,40)

// AFFICHERIEN,
// AFFICHEUSB,
// AFFICHESWEEP,
// AFFICHEPOTAR,
// AFFICHEMILIEU,
// AFFICHEADAFRUIT,
// AFFICHECLASSIQUE

{
    byte i;
    if (boutonP.hasBeenLongClicked())
    { 
        dateChangeServo=(millis());
        changeTypeServo=true;
        if (leServo.getType())
        {
            leServo.setType(false);
            affichage.affiche(AFFICHEADAFRUIT);
            Serial.println(F("servo de type Adafruit"));
        }
        else
        {
            leServo.setType(true);
            affichage.affiche(AFFICHECLASSIQUE);
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
}
*/

//------------------------------------------------------------------------------
//***************************** Set up *****************************************
void setup() 
{ 
    int vitesseDinit;    
    Serial.begin(57600);
    //potence.setPotence(); 
    lePotar.init();
    leServo.setType(true);
    EEPROM.get(ADRESSE_EEPROM, vitesseDinit);
    // Serial.print("vitesse lu=");Serial.println(vitesseDinit);
    if (vitesseDinit<=VITESSEMAXSERVO && vitesseDinit>=0){
        leServo.setVitesse(vitesseDinit);
        // Serial.println("lalalooo");
    } else {
        //Serial.println("lalalal");
        leServo.setVitesse(300);
    }

    Wire.begin();

    Serial.println("hello, wait to start !");
    //mySCoop.start();
    temp=millis();
    //etatCalibrateur=3; fait dans l'init de la variable
    etatCalibrateur = 8; //ne fait 
    //affichage.affiche(USB);
    attachInterrupt(0, updatePulseCompteur, CHANGE ); // Int0= la pin N°2 sur un UNO
}
void updatePulseCompteur(){ 
    compteur++;       
}     



//------------------------------------------------------------------------------
//***************************** Set up *****************************************
void loop ()
{
    byte i;
    switch (etatCalibrateur){
        case USB :
            //Serial.println(F(\
            //"Double Clic pour procéder à mesure via USB, sinon simple clic change de mode"));                              
            //etatCalibrateur=ETALORSWEEP;
            etatCalibrateur=MESREFPULSE;
            break;

        case ETALORSWEEP :
            if (boutonP.hasBeenClicked()){
                boutonP.acquit();
                etatCalibrateur=SWEEPTOMIN;
                affichage.affiche(SWEEPTOMIN);
                leServo.setObjectif(leServo.getMin());
                lePotar.acquit();
            }
            
            if (boutonP.hasBeenDoubleClicked()){
                boutonP.acquit();
                leServo.setVitesse(VITESSEMAXSERVO);
                leServo.setObjectif(leServo.getMilieu());
                // pour etre certain qu'on part du milieu et que le servo 
                //n'est pas n'importe ou suite à des manip de type sweep
                //ou potar par ex
                //etatCalibrateur=LECTURE_OU_ECRITURE_EEEPROM;
                etatCalibrateur=MESREFPULSE;
            } 
            break;   

        case MESREFPULSE:
            //mesure des impulsion de reference sur une rotation
            //de delta faite X fois pour plus de certitude
            compteurRef=0;
            compteur=0;
            for (i=0;i<NBRCYCLE_AR;i=i+1)
            {
                Serial.print(F("iteration de mesure de ref="));
                leServo.setObjectif(leServo.getMilieu()+DELTA);
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
                mesureRef[i]=compteur;
                leServo.setObjectif(leServo.getMilieu());
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
                compteur=0;
                Serial.println(mesureRef[i]);
            }
            etatCalibrateur=FINDMINMAX;
            compteurRef=moyenne(NBRCYCLE_AR,mesureRef);
            Serial.print(F(" SUPERcmpteurRefMoyenne="));
            Serial.print(compteurRef);
            Serial.print(F(" (ecart type : "));
            Serial.println(ecartType(NBRCYCLE_AR,mesureRef));
            if (compteurRef==0)
            {
                Serial.println(F("PB : compteur Ref=0 aucune impulsion fourche optique détecté"));
                Serial.println(F("retour au début de procédure"));
                etatCalibrateur=USB;
            }
            if (ERREURCOMPTAGE*3>compteurRef)
            {
                Serial.println(F("ce serait bien que la marge d'erreur soit de l'ordre de 30% de la mesure de reference"));
                Serial.println(F("je vais faire autre chose.... Programme a recompiler"));
                Serial.println(F("modifier macro ERREURCOMPTAGE et DELTA"));
                etatCalibrateur=POTAR;
                affichage.affiche(POTAR);
            }
            break;
        case FINDMINMAX: 
            //Serial.println("lalalal");
            //sequence pour aller au max puis au min et de déterminer
            //les pwm Min et Max du servo
            //----------------------------------------------------------
            //find max
            compteur=compteurRef;
            while (compteur>compteurRef-ERREURCOMPTAGE\
            && compteur<compteurRef+ERREURCOMPTAGE)
            {
                Serial.print("M+.....");
                compteur=0;
                leServo.setObjectif(leServo.getObjectif()+DELTA);
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
                Serial.print(" cmptr="); Serial.println(compteur);
            }
            leServo.setMax(leServo.getObjectif()-DELTA);
            leServo.setObjectif(leServo.getMilieu());
            Serial.print("MAX="); Serial.println(leServo.getMax());
            Serial.println("retourneMilieu");
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
            //----------------------------------------------------------
            //find mmin
            compteur=compteurRef;
            while (compteur>compteurRef-ERREURCOMPTAGE\
            && compteur<compteurRef+ERREURCOMPTAGE  )
            {
                Serial.print(F("M-......"));
                compteur=0;
                leServo.setObjectif(leServo.getObjectif()-DELTA);
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif(); 
                Serial.print(" cmpteur="); Serial.println(compteur);
            }
            leServo.setMin(leServo.getObjectif()+DELTA);
            leServo.setObjectif(leServo.getObjectif()+DELTA);
            Serial.print("MIN="); Serial.println(leServo.getMin());
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
            Serial.println("la le servo est au min et va faire grande course");
            leServo.setObjectif(leServo.getMin());
            //----------------------------------------------------------
            //séquence pour mesurer une vitesse et amplitude moyenne
            //(10 cycles d'aller et retour de min à max
            //et enregistrement systématique des mesures)
            for (i=0;i<NBRCYCLE_AR;i=i+1)
            {
                compteur=0;
                compteurRef=0;
                //compteur ref utilisé différement dans la suite 
                //par rapport à ce qui était fait jusque là. Compteur 
                //ref est utilisé pour voir si le servo continue
                //de bouger
                tempsMesureVitesse=millis();
                Serial.print("tempsMesureVitesse=");
                Serial.println(tempsMesureVitesse);
                // Mesure de min vers max
                leServo.setObjectif(leServo.getMax());
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
                Serial.print("cmpt="); Serial.println(compteur);
                while (compteur>compteurRef+20)
                {
                    compteurRef=compteur;   
                    Serial.print("yop");
                    Serial.print(" cmpt=");
                    Serial.println(compteur);
//sleep(TEMPOSLEEP);
                    leServo.appliqueObjectif();
                }
                resultatImpulsionMax[i]=compteur;
                resultatTempsMax[i]=millis()-tempsMesureVitesse-\
                TEMPO_STAT;
                Serial.print(F("Cycle "));
                Serial.print(i);
                Serial.print(F(" imp= "));
                Serial.print(resultatImpulsionMax[i]);
                Serial.print(F(" temps= "));
                Serial.println(resultatTempsMax[i]);
                
                compteur=0;
                compteurRef=0;
                tempsMesureVitesse=millis();
                // Mesure de max vers min
                leServo.setObjectif(leServo.getMin());
//sleep(TEMPOSLEEP);
                leServo.appliqueObjectif();
                while (compteur>compteurRef+20)
                {
                    compteurRef=compteur;
                    Serial.print("yup ");
                    Serial.print("cmpt=");
                    Serial.println(compteur);
//sleep(TEMPOSLEEP);
                    leServo.appliqueObjectif();
                }
                resultatImpulsionMin[i]=compteur;
                resultatTempsMin[i]=millis()-tempsMesureVitesse-TEMPO_STAT;
                Serial.print(F("Cycle "));
                Serial.print(i+1);
                Serial.print(F(" imp= "));
                Serial.print(resultatImpulsionMin[i]);
                Serial.print(F(" temps= "));
                Serial.println(resultatTempsMin[i]);
            } //fin for NBCYCLES

            
            
            //Serial.println(F("ATTENTION regarder  et noter le sens de rotation du premier mouvement "));
            //Serial.println(F("si pret faire un clic "));
            Serial.println(F("Fin"));
            //etatCalibrateur=WAITCLIC;
            etatCalibrateur = 8 ;
            
            
        case WAITCLIC:
            if (boutonP.hasBeenClicked())
            {
                boutonP.acquit();
                etatCalibrateur=DISPRESULT;
                leServo.setObjectif(leServo.getMax());
            } 
            break;

        case DISPRESULT:  
            Serial.println();
            Serial.println();
            Serial.println(F("Alors quelle sens de rotation ?") ); 
            Serial.println();
            Serial.println();
            Serial.print(F(" Dans le sens croissant pwm, moyenne : "));
            Serial.print(moyenne(NBRCYCLE_AR,resultatImpulsionMax));
            Serial.print(F(" impulsions (Ecart type : "));
            Serial.print(ecartType(NBRCYCLE_AR,resultatImpulsionMax));
            Serial.print(F(") atteintes en un temps moyen de : "));
            Serial.print(moyenne(NBRCYCLE_AR,resultatTempsMax));
            Serial.print(F(" ms (ecart type : "));
            Serial.print(ecartType(NBRCYCLE_AR,resultatTempsMax));
            Serial.println(F(" ms)"));
            Serial.print(F(" Dans le sens decroissant pwm, moyenne  : "));
            Serial.print(moyenne(NBRCYCLE_AR,resultatImpulsionMin));
            Serial.print(F("impulsions (Ecart type : "));
            Serial.print(ecartType(NBRCYCLE_AR,resultatImpulsionMin));
            Serial.print(F(") atteintes en un temps moyen de : "));
            Serial.print(moyenne(NBRCYCLE_AR,resultatTempsMin));
            Serial.print(F(" ms (ecart type : "));
            Serial.print(ecartType(NBRCYCLE_AR,resultatTempsMin));
            Serial.println(F(" ms)"));
            //potence.setObjectif(potence.getMax());
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
                leServo=servoTest(PINSERVO);
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
                EEPROM.put(ADRESSE_EEPROM, lePotar.getValue());
                Serial.print(F("sauver en prom vitesse = "));
                Serial.print( lePotar.getValue());
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
    delay(50);
    boutonP.refreshBouton();
    leServo.appliqueObjectif();
    
}

void serialEvent(){
    while (Serial.available() ) Serial.read();
    etatCalibrateur = USB;
}