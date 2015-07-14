#include <HamShield.h>
#include <Wire.h>

HamShield radio;

void setup() {
   Serial.begin(9600);
  Wire.begin();
  radio.initialize();
  radio.setFrequency(145050);
  radio.prepareTone1();
  radio.generateTone1(1200);
  radio.setModeTransmit();
  delay(5000);
  radio.generateTone1(2200);
  delay(5000);
  radio.setModeReceive();
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
