/*-------------------------------------------------------------------
fichier: robot_5eme
nom: Nasir Afzal
Hardware: feather m0 express + ecran feather + driver motor + capteur ultrason HC-SR04
date: 09-04-2025
---------------------------------------------------------------------*/

//------------------------------------------------------------------
// Include libraries
#include <Adafruit_SH110X.h>  // Librairie de l'écran
//------------------------------------------------------------------

//------------------------------------------------------------------
// Object definitions
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);  // Définition de l'objet "display" 128x64 points
//------------------------------------------------------------------

//------------------------------------------------------------------
// Pin definitions
#define trig_pin_G A0
#define echo_pin_G A1
#define trig_pin_D A2
#define echo_pin_D A3

// Button pins
#define BP_B 6            
#define BP_C 5            
#define BP_A 9 

// Motor pins
#define In1 10
#define In2 11
#define In3 12
#define In4 13

//------------------------------------------------------------------
// Global variables
unsigned long impulsion_G, impulsion_D;
unsigned int distance_G, distance_D;
int vitesse = 100, Mode = 0, Obstacle = 5;

//------------------------------------------------------------------
// Motor control functions
void avancer() {
  analogWrite(In1, vitesse);  // moteur gauche avant
  digitalWrite(In2, LOW);

  analogWrite(In3, vitesse);  // moteur droit avant
  digitalWrite(In4, LOW);
}

void reculer() {
  analogWrite(In1, 0);         // moteur gauche arrière
  digitalWrite(In2, HIGH);

  analogWrite(In3, 0);         // moteur droit arrière
  digitalWrite(In4, HIGH);
}

void gauche() {
  analogWrite(In1, vitesse);         // moteur gauche recule
  digitalWrite(In2, LOW);

  analogWrite(In3, vitesse);   // moteur droit avance
  digitalWrite(In4, HIGH);
}

void droite() {
  analogWrite(In1, 0);   // moteur gauche avance
  digitalWrite(In2, HIGH);

  analogWrite(In3, vitesse);         // moteur droit recule
  digitalWrite(In4, LOW);
}

void arret() {
  analogWrite(In1, 0);
  digitalWrite(In2, LOW);

  analogWrite(In3, 0);
  digitalWrite(In4, LOW);
}


//------------------------------------------------------------------
void setup() {
  Serial.begin(9600);      // Moniteur série
  Serial1.begin(9600);     // Communication avec le HC-08 (via Serial1)

  pinMode(trig_pin_D, OUTPUT);
  pinMode(echo_pin_D, INPUT);

  pinMode(trig_pin_G, OUTPUT);
  pinMode(echo_pin_G, INPUT);

  pinMode(BP_A, INPUT_PULLUP); 
  pinMode(BP_B, INPUT_PULLUP);  
  pinMode(BP_C, INPUT_PULLUP);

  pinMode(In1, OUTPUT); // Motor control pins
  pinMode(In2, OUTPUT);
  pinMode(In3, OUTPUT); 
  pinMode(In4, OUTPUT);

  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();
  display.setRotation(1); 
  display.setTextSize(1); 
  display.setTextColor(SH110X_WHITE);

  // Menu principal pour sélectionner le mode
  while (Mode == 0) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Mode BT");
    display.setCursor(0, 50);
    display.println("Mode auto");
    display.display();

    if (digitalRead(BP_A) == LOW) {
      Mode = 1; // Mode Bluetooth
      display.clearDisplay();
      display.setCursor(40, 30);
      display.println("Mode BT");
      display.display();
      delay(500);
    } 
    else if (digitalRead(BP_C) == LOW) {
      Mode = 2; // Mode automatique
      display.clearDisplay();
      display.setCursor(40, 30);
      display.println("Mode auto");
      display.display();
      delay(500);
    }
  }

  // Réglage de la vitesse si mode auto ou mode BT
  if (Mode == 2 || Mode == 1) { 
    while (true) {
      display.clearDisplay();
      display.setCursor(0, 10);
      display.println("+");

      display.setCursor(0, 50);
      display.println("-");

      display.setCursor(40, 30);
      display.printf("Vitesse: %d", vitesse);

      display.setCursor(0, 30);
      display.println("OK");
      display.display();

      if (digitalRead(BP_A) == LOW) { // Bouton A pour augmenter la vitesse
        vitesse++;
      } 
      else if (digitalRead(BP_C) == LOW) { // Bouton C pour diminuer la vitesse
        vitesse--;
      } 
      else if (digitalRead(BP_B) == LOW) { // Bouton B pour valider et sortir
        break;
      }

      delay(50);
    }
  }
}

//------------------------------------------------------------------
void loop() {
  if (Mode == 2) {
    // ------ Mode automatique -------
    display.clearDisplay();

    // Lecture des capteurs
    digitalWrite(trig_pin_D, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin_D, LOW);
    impulsion_D = pulseIn(echo_pin_D, HIGH, 30000);
    distance_D = (impulsion_D * 0.034) / 2;

    digitalWrite(trig_pin_G, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig_pin_G, LOW);
    impulsion_G = pulseIn(echo_pin_G, HIGH, 30000);
    distance_G = (impulsion_G * 0.034) / 2;

    // Affichage
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.printf("distance G : %d cm\n", distance_G);
    display.printf("distance D : %d cm\n", distance_D);
    display.printf("mode: %d vitesse: %d\n", Mode, vitesse);
    display.display();

    // Détection obstacle
    if (distance_G < Obstacle && distance_D >= Obstacle) {
      droite();
    } else if (distance_D < Obstacle && distance_G >= Obstacle) {
      gauche();
    } else if (distance_D < Obstacle && distance_G < Obstacle) {
      droite();
    } else {
      avancer();
    }
  }

  else if (Mode == 1) {
    // ------ Mode Bluetooth -------
    if (Serial1.available()) {
      char commande = Serial1.read();

      Serial.print("Commande reçue : ");
      Serial.println(commande);

      // Affichage de la commande
      display.clearDisplay();
      display.setCursor(10, 30);
      display.setTextSize(2);
      display.printf("Cmd: %c", commande);
      display.display();

      // Traitement
      switch (commande) {
        case 'U': avancer(); break;
        case 'D': reculer(); break;
        case 'L': gauche(); break;
        case 'R': droite(); break;
        case 'A': arret(); break;
        default: arret(); break;
      }
    }
  }
}
