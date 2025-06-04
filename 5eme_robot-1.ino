/*-------------------------------------------------------------------
fichier: robot_5eme
nom: Nasir Afzal
Matériel: Feather M0 Express + écran OLED Feather + driver moteur + capteur ultrason HC-SR04
date: 09-04-2025
---------------------------------------------------------------------*/

//------------------------------------------------------------------
// Inclusion des bibliothèques nécessaires
#include <Adafruit_SH110X.h>  // Librairie pour l'écran OLED
//------------------------------------------------------------------

//------------------------------------------------------------------
// Définition de l'objet "display" pour contrôler l'écran OLED
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);  // Initialisation de l'écran OLED 128x64 pixels
//------------------------------------------------------------------

//------------------------------------------------------------------
// Définition des broches (pins) utilisées pour les capteurs et actionneurs
#define trig_pin_G A0  // Broche de déclenchement du capteur ultrason gauche
#define echo_pin_G A1  // Broche d'écho du capteur ultrason gauche
#define trig_pin_D A2  // Broche de déclenchement du capteur ultrason droit
#define echo_pin_D A3  // Broche d'écho du capteur ultrason droit

// Broches des boutons
#define BP_B 6  // Bouton B
#define BP_C 5  // Bouton C
#define BP_A 9  // Bouton A

// Broches des moteurs
#define In1 10  // Contrôle du moteur gauche (avant)
#define In2 11  // Contrôle du moteur gauche (arrière)
#define In3 12  // Contrôle du moteur droit (avant)
#define In4 13  // Contrôle du moteur droit (arrière)

//------------------------------------------------------------------
// Variables globales
unsigned long impulsion_G, impulsion_D;  // Durée des impulsions des capteurs ultrasons
unsigned int distance_G, distance_D;     // Distances mesurées par les capteurs ultrasons
int vitesse = 100, Mode = 0, Obstacle = 5;  // Vitesse initiale, mode de fonctionnement, seuil d'obstacle
//------------------------------------------------------------------

//------------------------------------------------------------------
// Fonctions de contrôle des moteurs
void avancer() {
  analogWrite(In1, vitesse);  // Active le moteur gauche pour avancer
  digitalWrite(In2, LOW);

  analogWrite(In3, vitesse);  // Active le moteur droit pour avancer
  digitalWrite(In4, LOW);
}

void reculer() {
  analogWrite(In1, 0);        // Désactive le moteur gauche pour reculer
  digitalWrite(In2, HIGH);

  analogWrite(In3, 0);        // Désactive le moteur droit pour reculer
  digitalWrite(In4, HIGH);
}

void gauche() {
  analogWrite(In1, vitesse);  // Fait tourner le moteur gauche vers l'arrière
  digitalWrite(In2, LOW);

  analogWrite(In3, vitesse);  // Fait tourner le moteur droit vers l'avant
  digitalWrite(In4, HIGH);
}

void droite() {
  analogWrite(In1, 0);        // Fait tourner le moteur gauche vers l'avant
  digitalWrite(In2, HIGH);

  analogWrite(In3, vitesse);  // Fait tourner le moteur droit vers l'arrière
  digitalWrite(In4, LOW);
}

void arret() {
  analogWrite(In1, 0);        // Arrête le moteur gauche
  digitalWrite(In2, LOW);

  analogWrite(In3, 0);        // Arrête le moteur droit
  digitalWrite(In4, LOW);
}
//------------------------------------------------------------------

//------------------------------------------------------------------
void setup() {
  Serial.begin(9600);      // Initialise la communication série pour le moniteur série
  Serial1.begin(9600);     // Initialise la communication série pour le module Bluetooth HC-08

  // Configuration des broches des capteurs ultrasons
  pinMode(trig_pin_D, OUTPUT);
  pinMode(echo_pin_D, INPUT);

  pinMode(trig_pin_G, OUTPUT);
  pinMode(echo_pin_G, INPUT);

  // Configuration des broches des boutons
  pinMode(BP_A, INPUT_PULLUP); 
  pinMode(BP_B, INPUT_PULLUP);  
  pinMode(BP_C, INPUT_PULLUP);

  // Configuration des broches des moteurs
  pinMode(In1, OUTPUT); 
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT); 
  pinMode(In4, OUTPUT);

  // Initialisation de l'écran OLED
  display.begin(0x3C, true);  // Adresse I2C de l'écran
  display.clearDisplay();
  display.display();
  display.setRotation(1);  // Rotation de l'écran
  display.setTextSize(1);  // Taille du texte
  display.setTextColor(SH110X_WHITE);  // Couleur du texte

  // Menu principal pour sélectionner le mode de fonctionnement
  while (Mode == 0) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Mode BT");  // Option : Mode Bluetooth
    display.setCursor(0, 50);
    display.println("Mode auto");  // Option : Mode automatique
    display.display();

    // Sélection du mode via les boutons
    if (digitalRead(BP_A) == LOW) {  // Appui sur le bouton A pour le mode Bluetooth
      Mode = 1;  // Mode Bluetooth activé
      display.clearDisplay();
      display.setCursor(40, 30);
      display.println("Mode BT");
      display.display();
      delay(500);
    } 
    else if (digitalRead(BP_C) == LOW) {  // Appui sur le bouton C pour le mode automatique
      Mode = 2;  // Mode automatique activé
      display.clearDisplay();
      display.setCursor(40, 30);
      display.println("Mode auto");
      display.display();
      delay(500);
    }
  }

  // Réglage de la vitesse si le mode automatique ou Bluetooth est sélectionné
  if (Mode == 2 || Mode == 1) { 
    while (true) {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("+");  // Augmenter la vitesse

      display.setCursor(0, 50);
      display.println("-");  // Diminuer la vitesse

      display.setCursor(40, 30);
      display.printf("Vitesse: %d", vitesse);  // Affichage de la vitesse actuelle

      display.setCursor(0, 30);
      display.println("OK");  // Valider la vitesse
      display.display();

      // Modification de la vitesse via les boutons
      if (digitalRead(BP_A) == LOW) {  // Bouton A pour augmenter la vitesse
        vitesse++;
      } 
      else if (digitalRead(BP_C) == LOW) {  // Bouton C pour diminuer la vitesse
        vitesse--;
      } 
      else if (digitalRead(BP_B) == LOW) {  // Bouton B pour valider et sortir
        break;
      }

      delay(50);  // Attente pour éviter les rebonds
    }
  }
}
//------------------------------------------------------------------

//------------------------------------------------------------------
void loop() {
  if (Mode == 2) {
    // ------ Mode automatique -------
    display.clearDisplay();

    // Lecture des distances des capteurs ultrasons
    digitalWrite(trig_pin_D, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin_D, LOW);
    impulsion_D = pulseIn(echo_pin_D, HIGH, 30000);
    distance_D = (impulsion_D * 0.034) / 2;  // Calcul de la distance pour le capteur droit

    digitalWrite(trig_pin_G, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin_G, LOW);
    impulsion_G = pulseIn(echo_pin_G, HIGH, 30000);
    distance_G = (impulsion_G * 0.034) / 2;  // Calcul de la distance pour le capteur gauche

    // Affichage des distances et de la vitesse sur l'écran
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.printf("distance G : %d cm\n", distance_G);  // Distance gauche
    display.printf("distance D : %d cm\n", distance_D);  // Distance droite
    display.printf("vitesse: %d\n", vitesse);  // Vitesse actuelle
    display.display();

    // Détection d'obstacles et prise de décision
    if (distance_G < Obstacle && distance_D >= Obstacle) {
      droite();  // Tourner à droite si obstacle à gauche
    } else if (distance_D < Obstacle && distance_G >= Obstacle) {
      gauche();  // Tourner à gauche si obstacle à droite
    } else if (distance_D < Obstacle && distance_G < Obstacle) {
      reculer();  // Reculer si obstacles devant
    } else {
      avancer();  // Avancer si aucune détection d'obstacle
    }
  }

  else if (Mode == 1) {
    // ------ Mode Bluetooth -------
    if (Serial1.available()) {
      char commande = Serial1.read();  // Lecture de la commande reçue via Bluetooth

      Serial.print("Commande reçue : ");
      Serial.println(commande);

      // Affichage de la commande sur l'écran
      display.clearDisplay();
      display.setCursor(10, 30);
      display.setTextSize(2);
      display.printf("Cmd: %c", commande);
      display.display();

      // Traitement de la commande
      switch (commande) {
        case 'U': avancer(); break;  // Avancer
        case 'D': reculer(); break;  // Reculer
        case 'L': gauche(); break;   // Tourner à gauche
        case 'R': droite(); break;   // Tourner à droite
        case 'A': arret(); break;    // Arrêter
        default: arret(); break;     // Par défaut, arrêt
      }
    }
  }
}
//------------------------------------------------------------------