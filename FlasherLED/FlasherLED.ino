
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
  void ChangeParameters(long on, long off)
  {
    onTime = on;
    offTime = off;
  }
};


Flasher ledActivite(13, 800, 400);
Flasher led1(3, 80, 40);

void setup() {
  pinMode(2, INPUT);
}

void loop() {
  ledActivite.Update();
  led1.Update();
  if (digitalRead(2) == HIGH)
  {
    digitalWrite(3, HIGH);
  }
  
  
  delay(20);
}
