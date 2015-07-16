#include <HamShield.h>
#include <Wire.h>
#include <KISS.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

HamShield radio;
DDS dds;
KISS kiss(&Serial, &radio, &dds);

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT); // Debug clockTick
  pinMode(5, OUTPUT); // Debug AFSK timer
  pinMode(6, OUTPUT); // Debug bits
  pinMode(7, OUTPUT); // Debug bits
  pinMode(11, INPUT);
  
  Serial.begin(9600);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.print("Start.");
  radio.initialize();
  radio.setVHF();
  //radio.setSQOff();
  radio.setFrequency(145010);
  radio.setRfPower(15);
  I2Cdev::writeWord(A1846S_DEV_ADDR_SENLOW, 0x30, 0x06);
  I2Cdev::writeWord(A1846S_DEV_ADDR_SENLOW, 0x30, 0x26);
  I2Cdev::writeWord(A1846S_DEV_ADDR_SENLOW, 0x44, 0x05FF);
<<<<<<< HEAD
  //0b0000011111111111);
=======
>>>>>>> upstream/master

  dds.start();
  radio.afsk.start(&dds);
  dds.setFrequency(0);
  dds.on();
  dds.setAmplitude(255);
}

void loop() {
  kiss.loop();
}

ISR(ADC_vect) {
  kiss.isr();
}
