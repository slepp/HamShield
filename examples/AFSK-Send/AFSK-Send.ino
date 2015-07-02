#include <HamShield.h>
#include <Wire.h>

HamShield radio;
DDS dds;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(10, LOW);

  dds.start();
  radio.afsk.start(&dds);
  Serial.println("Starting...");
}

void loop() {
  // put your main code here, to run repeatedly:
    AFSK::Packet *packet = AFSK::PacketBuffer::makePacket(22 + 32);
    packet->start();
    packet->appendFCS('V'<<1);
    packet->appendFCS('E'<<1);
    packet->appendFCS('6'<<1);
    packet->appendFCS('S'<<1);
    packet->appendFCS('L'<<1);
    packet->appendFCS('P'<<1);
    packet->appendFCS(0b11100000);
    packet->appendFCS('V'<<1);
    packet->appendFCS('A'<<1);
    packet->appendFCS('6'<<1);
    packet->appendFCS('G'<<1);
    packet->appendFCS('A'<<1);
    packet->appendFCS(' '<<1);
    packet->appendFCS(0b01100001 | (15 & 0xf) << 1);
    packet->appendFCS(0x03);
    packet->appendFCS(0xf0);
    packet->print("Hello ");
    packet->println(millis());
    packet->finish();
    
    bool ret = radio.afsk.putTXPacket(packet);
    if(radio.afsk.txReady())
      if(radio.afsk.txStart()) {
        Serial.println("PTT here.");
      }
    // Wait 2 seconds before we send our beacon again.
    Serial.println("tick");
    delay(2000);
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
  TIFR1 = _BV(ICF1); // Clear the timer flag
  digitalWrite(2, HIGH);
  dds.clockTick();
  radio.afsk.timer();
  digitalWrite(2, LOW);
}
