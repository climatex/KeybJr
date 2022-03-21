#ifndef IR_H
#define IR_H

#include <util/delay.h>

inline void TransmitStrobe() __attribute__((always_inline));
inline void TransmitBitOne() __attribute__((always_inline));
inline void TransmitBitZero() __attribute__((always_inline));

// 40kHz IR strobe with a duration of 62.5us
inline void TransmitStrobe()
{
  uint8_t count = 5;
  while (count)
  {
    count--;
    PORTD ^= 0x20;
   _delay_us(12.5);
  }
  
  PORTD ^= 0x20;
}

// Transmit logical 1 in a bi-phase 440us data pulse
inline void TransmitBitOne()
{
  PORTD ^= 0x40;    // Assert cable output during first half phase
  TransmitStrobe(); // Infrared strobe
  _delay_us(157.5); // 220 microseconds in total
  PORTD ^= 0x40;    // Deassert during second half phase
  _delay_us(220);   // and wait
}

// Transmit logical 0 in a bi-phase 440us data pulse
inline void TransmitBitZero()
{
  _delay_us(220);   // First half phase no change
  PORTD ^= 0x40;    // Assert
  TransmitStrobe();
  _delay_us(157.5); // At the end of phase
  PORTD ^= 0x40;    // deassert
}

// Transmit one whole byte over infrared and cable
void TransmitByte(uint8_t theByte)
{ 
  // start bit
  TransmitBitOne();
  
  // count of bits that were log. 1, used for parity
  uint8_t bitCount = 8;
  uint8_t oneBits = 0;
  
  while(bitCount)
  {
    bitCount--;
    
    if (theByte & 1)
    {
      oneBits++;
      TransmitBitOne();
    }
    else
    {
      TransmitBitZero();
    }
    
    theByte >>= 1;
  }  
 
  // odd parity bit - transmit zero if odd already, one otherwise
  if (oneBits & 1)
  {
    TransmitBitZero();
  }
  else
  {
    TransmitBitOne();
  }
  
  //eleven empty stop bits... just a delay here for the Jr CPU
  _delay_us(440 * 11);
}

#endif