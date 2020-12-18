
#include "EmonLib.h"             
EnergyMonitor emon1; 
           
#include <Wire.h>  
                      
#include <SoftwareSerial.h>
SoftwareSerial SIM900(7, 8);

#include <LiquidCrystal_I2C.h>           // 2 inclure la librairie "LiquidCrystal_I2C.h" 
LiquidCrystal_I2C lcd(0x3f, 16, 2);            // 2 Si Le LCD ne fonctionne pas, alors mettre // sur la ligne précédente et enlever les // sur cette ligne



int MP = (" ") ;
int relais  = 13 ;
float w_instantane_in = 0;            // 2 Creation de la variable flottante "puissance instantanée" qui rentre (en watt) initialisée à la valeur 0
float wh_cumule_in = 0;              // 2' Création de la variable flottante correspondant au Kilo Watt heure "consommé" cumulés initialisé à 0 float w_instantane_out = 0;           // 2 Creation de la variable flottante "puissance instantanée" qui sort (negative) en watt initialisée à la valeur 0
float wh_cumule_out = 0;             // 2' Création de la variable flottante correspondant au Kilo Watt heure "surproduction" cumulés initialisé à 0
float w_instantane_out = 0;            // 2 Creation de la variable flottante "puissance instantanée" qui sort (en watt) initialisée à la valeur 0
unsigned long previous_millis = 0;   // 2 création de la variable "previous_millis" qui garde en mémoire le temps qui s'écoule en millièmes de seconde"

String message = " "; //string we're sending
char incomingChar;

//-----------------------INITIALISATION DU PROGRAMME-------------------------------------------------

void setup()
{
  Serial.begin(9600);                 // 1 Création du port série pour que l'arduino puisse envoyer des infos vers l'ordinateur
  pinMode(relais, OUTPUT);

  emon1.voltage(2, 357, 1.7);         // 1 Initialisation du Voltage (Pin A2, Valeur à changer pour etalonnage (+/-357 pour 6v et +/- 190 pour 12v))
  emon1.current(1, 28.5);             // 1 Initialisation du Courant en ampère ( Pin A1, Valeur à changer lors de l'etalonnage)
  lcd.init();                         // 2 initialisation de l'afficheur LCD

  Serial.begin(19200);
  SIM900.begin(19200);
  delay(20000);
  Serial.print("SIM900 ready...");
  SIM900.print("AT+CMGF=1\r");
  delay(100);
  SIM900.println("AT + CMGS = \"+336.........\"");
  delay(100);
  SIM900.println("le module gsm est connecte");
  delay(100);
  SIM900.println((char)26);
  delay(100);
  SIM900.println();
  delay(5000);

}

//----------------------- DEMARRAGE DE LA BOUCLE----------------------------------------------------

void loop()
{
  float puissance_reelle = emon1.realPower ;
  float verif_voltage    = emon1.Vrms;        //1 creation de la variable "volts moyen" (mesurable avec un voltmètre pour l'etalonnage)
  float verif_ampere     = emon1.Irms;        //1 creation de la variable "Ampères Moyen" (mesurable avec une pince ampèremétrique pour l'etalonnage))

  emon1.calcVI(20, 2000);                     // 1 Demande a Emonlib de tout calculer,  (puissance relle, volts moyen, ampère moyen et facteur de puissance)

  //emon1.serialprint();                      // (1) Si on ecrit cette ligne , toutes les valeurs calculées precedemment sont envoyées vers l'ordinateur

  //--------------------------Etalonnage des volts et ampères sans LCD--------------------------------------

  Serial.print("Est-ce le bon voltage? ");      // 1 envoyer vers l'ordinateur le texte " Est-ce le bon voltage? "
  Serial.print(verif_voltage);                  // 1 envoyer vers l'ordinateur la valeur "verif_voltage (Vrms)"
  Serial.print(" V  ");                         // 1 envoyer vers l'ordinateur le caractère "V"
  Serial.print("Est-ce le bon ampérage? ");
  Serial.print(verif_ampere);                   // 1 envoyer vers l'ordinateur la valeur "verif_voltage (Vrms)"
  Serial.print(" A ");                          // 1 envoyer vers l'ordinateur le caractère "A"

  //----------------POUR AVOIR LES W, Wh  de l'élélectricité qui rentre et de l'électricité qui sort de ma maison------------------

  if (puissance_reelle >= 0)                      // 2 Si la puissance reelle est positive, (c'est que je consomme et qu'a priori il n'y a pas de soleil)
  {
    w_instantane_in = puissance_reelle;      // 2 alors on dit que la puissance instantanée entrante (in) est egale à la puissance reelle.
    w_instantane_out = 0;                     // 2 dans ces conditions de consommation (positive) , la valeur de la surproduction est nulle.

    wh_cumule_in = wh_cumule_in + puissance_reelle * (millis() - previous_millis) / 3600000  ;
    
    // 2 La valeur cumulée consommée (entrante) = La valeur cumulée consommée précédente,
    // 2 plus la puissance reelle multipliée par le temps écoulé entre millis et previous millis
    // 2 que divise 3600 (nb secondes / heure) et encore par 1000 car millis compte les millièmes de seconde


  }

  else                                             // 2 SINON (c'est que la puissance_reelle est négative)
  {
    w_instantane_in = 0;                       // 2 idem au dessus
    w_instantane_out = puissance_reelle;       // 2 idem au dessus

    wh_cumule_out = wh_cumule_out + puissance_reelle * (millis() - previous_millis) / 3600000  ;
  }
  previous_millis = millis ();


  lcd.backlight();                // 2 Allumer l'écran LCD
  lcd.clear();                    // 2 rafraichir l'écran LCD ( efface les données précédentes)
  //1ere ligne,ecriture des watt  et watts heures facturés
  lcd.setCursor(0, 0);               // 4 positionner le curseur sur la ligne 1 et à gauche
  lcd.print("In:");                   // 4 ecrire "In " pour comprendre qu'il s'agit de la consommation
  lcd.print(w_instantane_in, 0);     // 4 puis écrire la valeur positive de la consommation au compteur EDF avec 2 chiffre derière la virgule
  lcd.setCursor(8, 0);               // 4 rester sur la ligne 1 mais positionner le curseur sur le caractère 8
  lcd.print("W");
  lcd.print(wh_cumule_in, 1);
  lcd.print("Wh");

  //2eme ligne, ecriture des watt  et watts heures qui correspondent à la surproduction de l'installation photovolatïque
  lcd.setCursor(0, 1);               // 4 positionner le curseur sur la ligne 2 et à gauche
  lcd.print("Out:");                   // 4 ecrire "In " pour comprendre qu'il s'agit de la consommation
  lcd.print(w_instantane_out, 0);     // 4 puis écrire la valeur positive de la consommation au compteur EDF avec 2 chiffre derière la virgule
  lcd.setCursor(8, 1);               // 4 rester sur la ligne 1 mais positionner le curseur sur le caractère 8
  lcd.print("W");
  lcd.print(wh_cumule_out, 1);
  lcd.print("Wh");


  if (SIM900.available() > 0) {
    message = SIM900.readString();
    Serial.print(message);
    delay(10);
  }
     if (message.indexOf(String(float(MP = (" ") <=  20000)))){
  
    message += " Energie vendu : ";
    message += String(float( (MP * 3360)/20000 ));
    message += " Wh ";
  }   

  if  ( ( ((MP * 3360)/20000 ) -  (wh_cumule_in ))> 0 ) {
    
    message += "Energie Totale Disponible : ";
    message += String( float  ((MP * 3360)/20000 ) - (wh_cumule_in ));
    message += " Wh ";
 
    digitalWrite(relais, HIGH);
    Serial.print("le relais est ON");
    SIM900.print("AT+CMGF=1\r");
    delay(100);
    SIM900.println("AT + CMGS = \"+336.........\"");
    delay(100);
    SIM900.println("le relais est active");
    message += "kit solaire active";
    delay(100);
    SIM900.println((char)26);
    delay(100);
    SIM900.println();
    delay(5000);
    message += "Energie consommee actuelle est ";
    message += String( float (wh_cumule_in  ));
    message += "Wh";
 }
 
  if  ( ( ((MP * 3360)/20000 ) -  (wh_cumule_in ))<= 0 ) {
    
    message += "Energie Totale Disponible : ";
    message += String( float  ((MP * 3360)/20000 ) - (wh_cumule_in ));
    message += " Wh ";
    
    digitalWrite(relais , LOW);
    Serial.println("le relais est OFF");
    SIM900.print("AT+CMGF=1\r");
    delay(100);
    SIM900.println("AT + CMGS = \"+336..........\"");
    delay(100);
    SIM900.println("kit solaire est desactive");
    message += "kit solaire est desactive";
    delay(100);
    SIM900.println((char)26);
    delay(100);
    SIM900.println();
    delay(5000);
    
    message += "Energie consommee actuelle est: ";
    message +=  String( float (wh_cumule_in + puissance_reelle * (millis() - previous_millis) / 3600000 ));
    message += " Wh";
    
  }

 if (message.indexOf("Etat") >= 0) {

    message += "Energie Totale Disponible : ";
    message += String( float ( ((MP * 3360)/20000 ) - (wh_cumule_in + puissance_reelle * (millis() - previous_millis) / 3600000 )));
    message += " Wh ";
    
    SIM900.print("AT+CMGF=1\r");
    delay(100);
    SIM900.println("AT + CMGS = \"+336.........\"");
    delay(100);
    SIM900.println("le relais est");
    delay(100);
    message = "";
    SIM900.println((char)26);
    delay(100);
    SIM900.println();
    delay(5000);

  }


}

//Fin du programme
