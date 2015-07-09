#define DDS_REFCLK_DEFAULT 9600

#include <HamShield.h>
#include <Wire.h>

HamShield radio;
DDS dds;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  
  Serial.println(F("Radio test connection"));
  Serial.println(radio.testConnection(), DEC);
  Serial.println(F("Initialize"));
  delay(100);
  radio.initialize();
  radio.setSQOn();
  Serial.println(F("Frequency"));
  delay(100);
  radio.frequency(145050);
  Serial.print(F("Squelch(H/L): "));
  //radio.setSQHiThresh(9000);
  Serial.print(radio.getSQHiThresh());
  Serial.print(F(" / "));
  //radio.setSQLoThresh(800);
  Serial.println(radio.getSQLoThresh());
  /*uint16_t b[4];
  I2Cdev::readWord(A1846S_DEV_ADDR_SENLOW, 0x30, b);
  b[0] &= ~(_BV(9) | _BV(8) |_BV(0));
  b[0] |= _BV(5);
  //I2Cdev::writeWord(A1846S_DEV_ADDR_SENLOW, 0x30, 791); //(791 & ~(_BV(9) | _BV(8))) | _BV(9));
  Serial.println(b[0]);
  //I2Cdev::writeWord(A1846S_DEV_ADDR_SENLOW, 0x30, 800);*/
  radio.setVolume1(7);
  radio.setVolume2(7);
  /*radio.setSQOn();
  radio.setVHF();*/
  radio.setModeReceive();
  Serial.print(F("RX? "));
  Serial.println(radio.getRX());
  Serial.println(F("DDS Start"));
  delay(100);
  dds.start();
  Serial.println(F("AFSK start"));
  delay(100);
  radio.afsk.start(&dds);
  Serial.println(F("Starting..."));
  pinMode(11, INPUT); // Bodge for now, as pin 3 is hotwired to pin 11
  delay(100);
  dds.setAmplitude(255);
}

uint32_t last = 0;
void loop() {
   if(radio.afsk.decoder.read() || radio.afsk.rxPacketCount()) {
      // A true return means something was put onto the packet FIFO
      Serial.println("Packet");
      // If we actually have data packets in the buffer, process them all now
      while(radio.afsk.rxPacketCount()) {
        AFSK::Packet *packet = radio.afsk.getRXPacket();
        if(packet) {
          for(unsigned short i = 0; i < packet->len; ++i)
            Serial.write((uint8_t)packet->getByte());
          AFSK::PacketBuffer::freePacket(packet);
        }
      }
    }
    /*if(last < millis()) {
      uint16_t buf;
      Serial.println(I2Cdev::readWord(A1846S_DEV_ADDR_SENLOW, A1846S_RSSI_REG, &buf));
      Serial.println(buf);
      last = millis()+1000;
    }*/
}

/*ISR(TIMER2_OVF_vect) {
  TIFR2 = _BV(TOV2);
  static uint8_t tcnt = 0;
  if(++tcnt == 8) {
  digitalWrite(2, HIGH);
  dds.clockTick();
  digitalWrite(2, LOW);
    tcnt = 0;
  }
}*/
ISR(ADC_vect) {
  static uint8_t tcnt = 0;
  TIFR1 = _BV(ICF1); // Clear the timer flag
  PORTD |= _BV(2); // Diagnostic pin (D2)
  //dds.clockTick();
  radio.afsk.timer();
  PORTD &= ~(_BV(2)); // Pin D2 off again
}

