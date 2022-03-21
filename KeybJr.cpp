/* Pin assignments as follows:
   PD2 (ATmega328 pin 4)  - Keyboard clock input
   PD3 (   -||-   pin 5)  - Keyboard data input
   PD4 (   -||-   pin 6)  - Keyboard select (AT/XT) input
   PD5 (   -||-   pin 11) - PCjr infrared output
   PD6 (   -||-   pin 12) - PCjr cable output
 */

#include <Arduino.h>
#include "IR.h"
#include "KeybXtAt.h"
#include "KeybPcJr.h"

// Global variable - using AT or XT keyboard depending on jumper
uint8_t usingXtKeyboard = 0;

void setup()
{ 
  // Normal behavior: PD5 + PD6 outputs, PD4 + PD3 + PD2 as inputs
  DDRD = 0xe2;
  
  // By default, keybselect, keybclock and Jr cable output pulled up...
  PORTD |= 0x54;
  // .. and the infrared pulled down
  PORTD &= 0xDF;
  
  // If using XT keyboard - PD4 jumpered to GND
  usingXtKeyboard = !((PIND >> 4) & 1);
  
  // Initialize chosen keyboard  
  InitializeKeyboard();
}

void loop()
{
  // Get a scancode from the keyboard and convert it to XT (if on AT/PS2 keyboard).  
  // Then convert it for PCjr finesse and transmit through infrared and cable.
  // During TX, the keyboard waits.
  
  const uint8_t scanCode = ReadKeyboard();
  DisableKeyboard();
  SendScancode(scanCode);
  EnableKeyboard();
}