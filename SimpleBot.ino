#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Wire.h>
#include <LSM303.h>

//Przypisanie pinow
const int trigPin = 2;
const int echoPin = 4;
const int servoPin = 5;

//zmienne
uint16_t volatile servoTime = 0;
uint16_t volatile servoHighTime = 3000;
boolean volatile servoHigh = false;
long czas, cm;
int dl, d, dr;

//tworzenie obiektow
ZumoMotors motors;
ZumoBuzzer buzzer;
const char sound_effect[] PROGMEM = "O4 T100 V15 L4 MS g12>c12>e12>G6>E12 ML>G2";

void setup() {
  //rozpoczyna komunikacje
  Serial.begin(9600);
  
  //serwo
  servoInit();
  
  //przypisanie pinow
  pinMode(13, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  buzzer.playMode(PLAY_AUTOMATIC);
  buzzer.playFromProgramSpace(sound_effect);
  
  //ustawienie poczatkowe
  rozejrzyj();
}

void loop() {
 rozejrzyj();
 delay(3000); 
}

//przeliczanie milisekund na odleglosc
long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

//odleglosc
int sprawdzOdleglosc() {
  for (int i = 0; i<5; i++ ) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    czas = pulseIn(echoPin, HIGH);
    cm =  microsecondsToCentimeters(czas);
  }
  Serial.println(cm);
  return cm;
}

//sprawdz w ktora strone
void rozejrzyj() {
  servoSetPosition(1500);
  delay(1000);
  d = sprawdzOdleglosc();
  delay(1000);
  servoSetPosition(2500);
  delay(1000);
  dl = sprawdzOdleglosc();
  delay(1000);
  servoSetPosition(575);
  delay(1000);
  dr = sprawdzOdleglosc();
  delay(1000);
  servoSetPosition(1500);
}

//funkcje do obslugi serwa
ISR(TIMER2_COMPA_vect) {
  servoTime += OCR2A + 1;
  static uint16_t highTimeCopy = 3000;
   static uint8_t interruptCount = 0;
   if(servoHigh)
   {
     if(++interruptCount == 2)
{
OCR2A = 255;
}
     if(servoTime >= highTimeCopy) {
       // The pin has been high enough, so do a falling edge.
       digitalWrite(servoPin, LOW);
       servoHigh = false;
       interruptCount = 0;
      }
}
else
   {
     // The servo pin is currently low.
     if(servoTime >= 40000)
     {
       // We've hit the end of the period (20 ms),
       // so do a rising edge.
       highTimeCopy = servoHighTime;
       digitalWrite(servoPin, HIGH);
       servoHigh = true;
       servoTime = 0;
       interruptCount = 0;
       OCR2A = ((highTimeCopy % 256) + 256)/2 - 1;
} 
}
}
  
  void servoInit()
 {
   digitalWrite(servoPin, LOW);
   pinMode(servoPin, OUTPUT);
   // Turn on CTC mode.  Timer 2 will count up to OCR2A, then
   // reset to 0 and cause an interrupt.
   TCCR2A = (1 << WGM21);
   // Set a 1:8 prescaler.  This gives us 0.5us resolution.
   TCCR2B = (1 << CS21);
   // Put the timer in a good default state.
   TCNT2 = 0;
   OCR2A = 255;
   TIMSK2 |= (1 << OCIE2A);  // Enable timer compare interrupt.
   sei();   // Enable interrupts.
 }
 void servoSetPosition(uint16_t highTimeMicroseconds)
 {
   TIMSK2 &= ~(1 << OCIE2A); // disable timer compare interrupt
   servoHighTime = highTimeMicroseconds * 2;
   TIMSK2 |= (1 << OCIE2A); // enable timer compare interrupt
}

