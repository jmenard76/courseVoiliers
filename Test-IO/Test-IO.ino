/*
   Programme de test des entrées/sorties pour le jeu de course de voiliers
*/

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

Flasher led1(47, 800, 400);
Flasher led2(23, 800, 400);
Flasher led3(31, 800, 400);
Flasher led4(39, 800, 400);
int lastState[50];

void setup() {
  pinMode(22, INPUT);
  pinMode(24, INPUT);
  pinMode(26, INPUT);
  pinMode(28, INPUT);
  pinMode(30, INPUT);
  pinMode(32, INPUT);
  pinMode(34, INPUT);
  pinMode(36, INPUT);
  pinMode(38, INPUT);
  pinMode(40, INPUT);
  pinMode(42, INPUT);
  pinMode(44, INPUT);
  pinMode(46, INPUT);

  Serial.begin(9600);
  delay(500);
}

void loop() {
  for (int i = 22; i <= 46; i = i + 2) {
    if ((digitalRead(i) == HIGH) && (lastState[i] == LOW))
    {
      Serial.print("Entrée n° ");
      Serial.println(i);
      lastState[i] = HIGH;
    }
    if (digitalRead(i) == LOW)
    {
      lastState[i] = LOW;
    }
  }
  led1.Update();
  led2.Update();
  led3.Update();
  led4.Update();
}
