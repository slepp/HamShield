#include <HamShield.h>
#include "AFSK.h"
#include "KISS.h"

#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;
static AFSK::Packet *inPacket = NULL;
static bool prevFEND = false; // If our previous input was a FEND

// Inside the KISS loop, we basically wait for data to come in from the
// KISS equipment, and look if we have anything to relay along
void KISS::loop() {
  static bool currentlySending = false;
  if(radio->afsk.decoder.read() || radio->afsk.rxPacketCount()) {
     // A true return means something was put onto the packet FIFO
     // If we actually have data packets in the buffer, process them all now
     while(radio->afsk.rxPacketCount()) {
       AFSK::Packet *packet = radio->afsk.getRXPacket();
       if(packet) {
         writePacket(packet);
         AFSK::PacketBuffer::freePacket(packet);
       }
     }
   }
   // Check if we have incoming data to turn into a packet
   while(io->available()) {
     uint8_t c = (uint8_t)io->read();
     // Receiving a FEND means we are either about to start or just ended
     if(c == KISS_FEND) {
       if(inPacket && inPacket->len) { // If we have a packet with length
         inPacket->finish(); // Append FCS
         inPacket->parsePacket();
         const AFSK::Packet *p = inPacket;
         lcd.setCursor(0,0);
         lcd.clear();
         lcd.print(p->srcCallsign);
         lcd.write('-');
         lcd.print(p->srcSSID);
         lcd.print(" L");
         lcd.print(p->len-2);
         lcd.setCursor(0, 1);
         lcd.print(p->dstCallsign);
         lcd.write('-');
         lcd.print(p->dstSSID);
         lcd.setCursor(10,1);
         lcd.print(p->control, HEX);
         lcd.print(p->pid, HEX);
         lcd.setCursor(15,1);
         lcd.print(">>");
         inPacket->restoreFCS();
         radio->afsk.encoder.putPacket(inPacket); // And queue it
         inPacket = NULL; // Go back out of frame
       }
       prevFEND = true;
     }
     // We're inside the boundaries of a FEND
     if(inPacket) {
       // Unescape the incoming data
       if(c == KISS_FESC) {
         c = io->read();
         if(c == KISS_TFESC) {
           c = KISS_FESC;
         } else {
           c = KISS_FEND;
         }
       }
       inPacket->appendFCS(c);
     }
     // Not in a packet, didn't get a FEND, and we had a FEND prior to this
     if(!inPacket && c != KISS_FEND && prevFEND) { // means we have our TNC byte
       if((c & 0xf) == 0) {
         // First byte<3:0> should be a 0, otherwise we're having options
         // We're expecting regular frame data here, so get a packet going
         inPacket = AFSK::PacketBuffer::makePacket(PACKET_MAX_LEN);
         // If alocation failed, inPacket == NULL, and we'll not start
         // accumulating frame bytes.
       } else {
         // Handle settings in the first byte
         uint8_t p = (uint8_t)io->read();
         switch((uint8_t)(c & 0xf)) {
           case 1: // TXDelay
             radio->afsk.encoder.setPreamble(p);
             break;
           case 2: // Persistence
             // TODO: This.
             break;
           case 3: // Slot time
             // TODO: This.
             break;
           case 5: // Full duplex
             // TODO: Perhaps this.
             break;
           default: // All other commands are just ignored.
             break;
         }
       }
       prevFEND = false;
     }
   }
   if(radio->afsk.txReady()) {
     radio->setModeTransmit();
     lcd.setCursor(16,0);
     lcd.write('T');
     currentlySending = true;
     if(!radio->afsk.txStart()) { // Unable to start for some reason
       radio->setModeReceive();
       lcd.setCursor(16,0);
       lcd.write('R');
       currentlySending = false;
     }
   }
   if(currentlySending && radio->afsk.encoder.isDone()) {
    radio->setModeReceive();
    lcd.setCursor(16,0);
    lcd.write('R');
    currentlySending = false;
  }
}

void KISS::writePacket(AFSK::Packet *p) {
  uint16_t i;
  p->parsePacket();
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print(p->srcCallsign);
  lcd.write('-');
  lcd.print(p->srcSSID);
  lcd.print(" L");
  lcd.print(p->len-2);
  lcd.setCursor(0, 1);
  lcd.print(p->dstCallsign);
  lcd.write('-');
  lcd.print(p->dstSSID);
  lcd.setCursor(10,1);
  lcd.print(p->control, HEX);
  lcd.print(p->pid, HEX);
  lcd.setCursor(15,1);
  lcd.print("<<");
  p->restoreFCS();
  io->write(KISS_FEND);
  io->write((uint8_t)0); // Host to TNC port identifier
  for(i = 0; i < p->len-2; i++) {
    uint8_t c = (uint8_t)p->getByte(i);
    if(c == KISS_FEND || c == KISS_FESC) {
      io->write(KISS_FESC);
      io->write((c==KISS_FEND?KISS_TFEND:KISS_TFESC));
    } else {
      io->write(c);
    }
  }
  io->write(KISS_FEND);
}
