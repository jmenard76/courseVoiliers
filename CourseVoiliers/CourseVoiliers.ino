/*   Course de Voiliers
     Jérôme Menard
     Version 1.1 - Mai 2018 : version finale.
     Version 1.2 - Mai 2019 : modification des adresses de sortie pour les LED suite à remplacement du cablage interne. Rempacement des FdC par des barrières lumineuses.
*/

#include <RedMP3.h>
#include <AFMotor.h>

// Tests
const bool DEBUG = false;

/*
   Gestion du son
*/
#define MP3_RX 18
#define MP3_TX 19
MP3 lecteurAudio(MP3_RX, MP3_TX);
int8_t volume = 0x14;     // 0~0x1e (niveau ajustable)
#define sonAvance 0x01    // 0x01 (vagues lorsqu'un voilier avance)
#define sonDepart 0x02    // 0x02 (ding lorsqu'un voilier retourne au départ)
#define sonArrivee 0x03   // 0x03 (corne lorsqu'un voilier arrive)
#define sonAttente 0x04   // 0x04 (attente)
#define sonAccueil 0x05   // 0x05 (accueil au démarrage)


/*
   Gestion des moteurs
*/
AF_DCMotor motor_V1(1, MOTOR12_8KHZ);
AF_DCMotor motor_V2(2, MOTOR12_8KHZ);
AF_DCMotor motor_V3(3, MOTOR34_8KHZ);


/*
   Clignotement d'une LED
   Paramètre onTime (0 : éteinte, -1 : allumée, >0 : clignote)
*/
class Flasher
{
    byte ledPin;
    long onTime;
    long offTime;
    int ledState;
    unsigned long previousMillis;
  public:
    Flasher(int pin, long on, long off)
    {
      ledPin = pin;
      pinMode(ledPin, OUTPUT);
      onTime = on;
      offTime = off;
      ledState = LOW;
      previousMillis = 0;
    }
    void Update()
    {
      if (onTime > 0)
      {
        unsigned long currentMillis = millis();
        if ((ledState == HIGH) && (currentMillis - previousMillis >= onTime))
        {
          ledState = LOW;
          previousMillis = currentMillis;
          digitalWrite(ledPin, ledState);
        }
        else if ((ledState == LOW) && (currentMillis - previousMillis >= onTime))
        {
          ledState = HIGH;
          previousMillis = currentMillis;
          digitalWrite(ledPin, ledState);
        }
      }
      else if (onTime == 0)
      {
        digitalWrite(ledPin, LOW);
      }
      else if (onTime == -1)
      {
        digitalWrite(ledPin, HIGH);
      }
    }
    void ChangeParameters(long on, long off)
    {
      onTime = on;
      offTime = off;
    }
};


/*
   Gestion des Voiliers
*/
class Voilier
{
    byte indexMotor;
    byte fdcDepartPin;
    byte fdcArriveePin;
    byte lastStateFdcArriveePin;
    byte fdcTrou1Pin;
    byte lastStateFdcTrou1Pin;
    byte fdcTrou2Pin;
    byte lastStateFdcTrou2Pin;
    int motorState1;
    int motorState2;
    int motorStateInit;
    unsigned long previousMillis;
  public:
    Voilier(byte index, byte pinDepart, byte pinArrivee, byte pinTrou1, byte pinTrou2)
    {
      indexMotor = index;
      fdcDepartPin = pinDepart;
      pinMode(fdcDepartPin, INPUT);
      fdcArriveePin = pinArrivee;
      pinMode(fdcArriveePin, INPUT);
      fdcTrou1Pin = pinTrou1;
      pinMode(fdcTrou1Pin, INPUT);
      fdcTrou2Pin = pinTrou2;
      pinMode(fdcTrou2Pin, INPUT);
      lastStateFdcTrou1Pin = LOW;
      lastStateFdcTrou2Pin = LOW;
      lastStateFdcArriveePin = LOW;
      motorState1 = LOW;
      motorState2 = LOW;
      motorStateInit = LOW;
    }
    void RetourAuDepart()
    {
      if ((motorStateInit == HIGH) && (digitalRead(fdcDepartPin) == HIGH))
      {
        motorStateInit = LOW;
        if (indexMotor == 1) {
          motor_V1.run(RELEASE);
        }
        else if (indexMotor == 2) {
          motor_V2.run(RELEASE);
        }
        else if (indexMotor == 3) {
          motor_V3.run(RELEASE);
        }
        lecteurAudio.playWithFileName(0x01, sonDepart);
      }
      else if ((motorStateInit == LOW) && (digitalRead(fdcDepartPin) == LOW))
      {
        motorStateInit = HIGH;
        if (indexMotor == 1) {
          motor_V1.run(BACKWARD);
        }
        else if (indexMotor == 2) {
          motor_V2.run(BACKWARD);
        }
        else if (indexMotor == 3) {
          motor_V3.run(BACKWARD);
        }
      }
    }
    int Avance(long onTime, int motorState)
    {
      unsigned long currentMillis = millis();
      if (((motorState == HIGH) && (currentMillis - previousMillis >= onTime)) || (digitalRead(fdcArriveePin) == HIGH))
      {
        motorState = LOW;
        if (indexMotor == 1) {
          motor_V1.run(RELEASE);
        }
        else if (indexMotor == 2) {
          motor_V2.run(RELEASE);
        }
        else if (indexMotor == 3) {
          motor_V3.run(RELEASE);
        }
        if (DEBUG) {
          Serial.println(currentMillis - previousMillis);
        }
      }
      else if ((motorState == LOW) && (digitalRead(fdcArriveePin) == LOW))
      {
        motorState = HIGH;
        previousMillis = currentMillis;
        if (indexMotor == 1) {
          motor_V1.run(FORWARD);
        }
        else if (indexMotor == 2) {
          motor_V2.run(FORWARD);
        }
        else if (indexMotor == 3) {
          motor_V3.run(FORWARD);
        }
        lecteurAudio.playWithFileName(0x01, sonAvance);
      }
      return motorState;
    }
    void Update()
    {
      if (motorStateInit == LOW) // Si le voilier n'est pas en train de revenir à sa position de départ
      {
        // Balle dans trou n°1
        if (((digitalRead(fdcTrou1Pin) == HIGH) && (lastStateFdcTrou1Pin == LOW)) || (motorState1 == HIGH))
        {
          motorState1 = Avance(500, motorState1);
          if ((DEBUG) && (digitalRead(fdcTrou1Pin) == HIGH) && (lastStateFdcTrou1Pin == LOW)) {
            Serial.println(fdcTrou1Pin);
          }
          lastStateFdcTrou1Pin = HIGH;
        }
        if (digitalRead(fdcTrou1Pin) == LOW) {
          lastStateFdcTrou1Pin = LOW;
        }
        // Balle dans trou n°2
        if (((digitalRead(fdcTrou2Pin) == HIGH) && (lastStateFdcTrou2Pin == LOW)) || (motorState2 == HIGH))
        {
          motorState2 = Avance(1000, motorState2);
          if ((DEBUG) && (digitalRead(fdcTrou2Pin) == HIGH) && (lastStateFdcTrou2Pin == LOW)) {
            Serial.println(fdcTrou2Pin);
          }
          lastStateFdcTrou2Pin = HIGH;
        }
        if (digitalRead(fdcTrou2Pin) == LOW) {
          lastStateFdcTrou2Pin = LOW;
        }
        // Joue le son d'arrivée
        if ((digitalRead(fdcArriveePin) == HIGH) && (lastStateFdcArriveePin == LOW))
        {
          if ((DEBUG) && (digitalRead(fdcArriveePin) == HIGH)) {
            Serial.println("Arrivée");
          }
          lecteurAudio.playWithFileName(0x01, sonArrivee);
          lastStateFdcArriveePin = HIGH;
        }
        if (digitalRead(fdcArriveePin) == LOW) {
          lastStateFdcArriveePin = LOW;
        }
      }
    }
    // Voilier à l'arrivée
    int EtatVictoire()
    {
      int arrivee = digitalRead(fdcArriveePin);
      return arrivee;
    }
    // Voilier au départ
    int EtatDepart()
    {
      int depart = digitalRead(fdcDepartPin);
      return depart;
    }
    // Voilier en retour vers le départ
    int EtatInit()
    {
      return motorStateInit;
    }
    // Déplacement en cours (pour l'animation de la LED d'activité)
    int EtatMouvement()
    {
      return (motorState1 || motorState2 || motorStateInit);
    }
};


/*
   Variables générales
*/
const byte pinBoutonInit = 46;
Voilier voilier1(1, 26, 28, 22, 24);
Voilier voilier2(2, 34, 36, 30, 32);
Voilier voilier3(3, 42, 44, 38, 40);
Flasher ledActivite(29, 800, 400);
Flasher ledArrivee_V1(23, 0, 0);
Flasher ledArrivee_V2(25, 0, 0);
Flasher ledArrivee_V3(27, 0, 0);
int indexVictoire = 0;


void setup() {
  delay(500);
  lecteurAudio.setVolume(volume);
  lecteurAudio.playWithFileName(0x01, sonAccueil);
  pinMode(pinBoutonInit, INPUT);
  if (DEBUG) {
    Serial.begin(9600);
  }
  // Activation des moteurs
  motor_V1.setSpeed(215);
  motor_V1.run(RELEASE);
  motor_V2.setSpeed(235);
  motor_V2.run(RELEASE);
  motor_V3.setSpeed(200);
  motor_V3.run(RELEASE);
  delay(6500);
}


void loop() {
  // Mettre les voiliers en position de départ
  if ((digitalRead(pinBoutonInit) == HIGH) || (voilier1.EtatInit() == HIGH) || (voilier2.EtatInit() == HIGH) || (voilier3.EtatInit() == HIGH))
  {
    if ((DEBUG) && (digitalRead(pinBoutonInit) == HIGH)) {
      Serial.println("Initialisation du jeu...");
    }
    voilier1.RetourAuDepart();
    voilier2.RetourAuDepart();
    voilier3.RetourAuDepart();
  }

  // Les 3 voiliers sont en position de départ
  if ((voilier1.EtatDepart() == HIGH) && (voilier2.EtatDepart() == HIGH) && (voilier3.EtatDepart() == HIGH))
  {
    if (DEBUG) {
      Serial.println("Prêt au départ");
    }
    indexVictoire = 0;
    ledActivite.ChangeParameters(800, 400);
    ledArrivee_V1.ChangeParameters(0, 0);
    ledArrivee_V2.ChangeParameters(0, 0);
    ledArrivee_V3.ChangeParameters(0, 0);
  }

  // Animation des LED d'arrivée
  if (voilier1.EtatVictoire() == HIGH) {
    ledArrivee_V1.ChangeParameters(800, 400);
  }
  if (voilier2.EtatVictoire() == HIGH) {
    ledArrivee_V2.ChangeParameters(800, 400);
  }
  if (voilier3.EtatVictoire() == HIGH) {
    ledArrivee_V3.ChangeParameters(800, 400);
  }

  // Quel voilier a gagné ?
  if (indexVictoire == 0)
  {
    if (voilier1.EtatVictoire() == HIGH) {
      indexVictoire = 1;
    }
    else if (voilier2.EtatVictoire() == HIGH) {
      indexVictoire = 2;
    }
    else if (voilier3.EtatVictoire() == HIGH) {
      indexVictoire = 3;
    }
  }
  else
    // LED d'arrivée fixe pour identifier le vainqueur
  {
    if (indexVictoire == 1) {
      ledArrivee_V1.ChangeParameters(-1, 0);
    }
    if (indexVictoire == 2) {
      ledArrivee_V2.ChangeParameters(-1, 0);
    }
    if (indexVictoire == 3) {
      ledArrivee_V3.ChangeParameters(-1, 0);
    }
  }

  // Animation LED activité
  if ((voilier1.EtatMouvement() == HIGH) || (voilier2.EtatMouvement() == HIGH) || (voilier3.EtatMouvement() == HIGH))
  {
    ledActivite.ChangeParameters(80, 40);
  }
  else
  {
    ledActivite.ChangeParameters(800, 400);
  }

  // Appel des sous-programmes Voiliers et LED
  voilier1.Update();
  voilier2.Update();
  voilier3.Update();

  ledActivite.Update();
  ledArrivee_V1.Update();
  ledArrivee_V2.Update();
  ledArrivee_V3.Update();

  // Petite attente pour alléger le processeur...
  delay(10);
}
