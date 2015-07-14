#ifndef _KISS_H_
#define _KISS_H_

#include <HamShield.h>
#include "AFSK.h"

#define KISS_FEND 0xC0
#define KISS_FESC 0xDB
#define KISS_TFEND 0xDC
#define KISS_TFESC 0xDD

class KISS {
public:
  KISS(Stream *_io, HamShield *h, DDS *d) : io(_io), radio(h), dds(d) {}
  bool read();
  void writePacket(AFSK::Packet *);
  void loop();
  inline void isr() { // Conditional defines for refclk's off the 9600 required
#if (DDS_REFCLK_DEFAULT != 9600)
    static uint8_t tcnt = 0;
#endif
    TIFR1 = _BV(ICF1); // Clear the timer flag
    PORTD |= _BV(4);
    dds->clockTick();
#if (DDS_REFCLK_DEFAULT != 9600)
    if(++tcnt == (DDS_REFCLK_DEFAULT/9600)) {
#endif
      PORTD |= _BV(5); // Diagnostic pin (D5)
      radio->afsk.timer();
#if (DDS_REFCLK_DEFAULT != 9600)
      tcnt = 0;
    }
#endif
    PORTD &= ~(_BV(4) | _BV(5));
  }
private:
  Stream *io;
  HamShield *radio;
  DDS *dds;
};

#endif /* _KISS_H_ */
