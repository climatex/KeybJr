#ifndef KEYBXTAT_H
#define KEYBXTAT_H

// Initialization inlined into the entry point
inline void InitializeKeyboard() __attribute__((always_inline));

// From setup()
extern uint8_t usingXtKeyboard;

/* ************************************ XT keyboard specific ************************************ */
volatile uint8_t xtKeybClock = 0;
volatile uint8_t xtKeybData = 0;
volatile uint8_t xtKeybAvailable = 0;
uint8_t xtKeybStartbitsCount = 1; // usually 1. Or 2 for [very] old XT keyboards

void XtKeyboardClockISR()
{   
  // Immediately read data bit from data pin!
  const uint8_t bit = (PIND >> 3) & 1;
     
  // Only perform OR if bit is one; skip start bits. One, or two
  if (bit && (xtKeybClock >= xtKeybStartbitsCount))
  {
    xtKeybData |= bit << (xtKeybClock - xtKeybStartbitsCount);
  }
  
  xtKeybClock++;
  
  // Read finished? Store into buffer
  if (xtKeybClock == 8 + xtKeybStartbitsCount)  
  {
    // Ignore data if something nonstandard
    const uint8_t scanCodeNoMSB = xtKeybData & 0x7f;
    
    if ((scanCodeNoMSB > 0) && (scanCodeNoMSB <= 0x53))
    {
      // Set data available
      xtKeybAvailable = xtKeybData;
    }
    
    // Reset data and counters
    xtKeybData = xtKeybClock = 0;    
  }
}

uint8_t XtKeyboardRead()
{
  // Wait for key - nonzero
  while(!xtKeybAvailable) {}
  
  uint8_t scanCode = xtKeybAvailable;
  xtKeybAvailable = 0;
  
  return scanCode;
}

/* ************************************ AT keyboard specific ************************************ */
#include "src/PS2KeyAdvanced-1.0.9/PS2KeyAdvanced.h"

PS2KeyAdvanced atKeyboard;
uint8_t atKeybLockKeys = 0;
uint8_t xlatTable[256];

void SetupAtToXtTable()
{
  // PS2 / AT key to XT scancode (make codes, i.e. when pressed)
  for (int i = 0; i < sizeof(xlatTable); i++)
  {
    xlatTable[i] = 0xff; // default: no match in table
  }
  
  xlatTable[PS2_KEY_ESC]=0x01;
  xlatTable[PS2_KEY_1]=0x02;
  xlatTable[PS2_KEY_2]=0x03;
  xlatTable[PS2_KEY_3]=0x04;
  xlatTable[PS2_KEY_4]=0x05;
  xlatTable[PS2_KEY_5]=0x06;
  xlatTable[PS2_KEY_6]=0x07;
  xlatTable[PS2_KEY_7]=0x08;
  xlatTable[PS2_KEY_8]=0x09;
  xlatTable[PS2_KEY_9]=0x0A;  
  xlatTable[PS2_KEY_0]=0x0B;
  xlatTable[PS2_KEY_MINUS]=0x0C;
  xlatTable[PS2_KEY_EQUAL]=0x0D;
  xlatTable[PS2_KEY_BS]=0x0E;
  xlatTable[PS2_KEY_TAB]=0x0F;
  xlatTable[PS2_KEY_Q]=0x10;
  xlatTable[PS2_KEY_W]=0x11;
  xlatTable[PS2_KEY_E]=0x12;
  xlatTable[PS2_KEY_R]=0x13;
  xlatTable[PS2_KEY_T]=0x14;
  xlatTable[PS2_KEY_Y]=0x15;
  xlatTable[PS2_KEY_U]=0x16;
  xlatTable[PS2_KEY_I]=0x17;
  xlatTable[PS2_KEY_O]=0x18;
  xlatTable[PS2_KEY_P]=0x19;
  xlatTable[PS2_KEY_OPEN_SQ]=0x1A;
  xlatTable[PS2_KEY_CLOSE_SQ]=0x1B;
  xlatTable[PS2_KEY_ENTER]=0x1C;
  xlatTable[PS2_KEY_KP_ENTER]=0x1C;
  xlatTable[PS2_KEY_L_CTRL]=0x1D;
  xlatTable[PS2_KEY_R_CTRL]=0x1D;
  xlatTable[PS2_KEY_A]=0x1E;
  xlatTable[PS2_KEY_S]=0x1F;
  xlatTable[PS2_KEY_D]=0x20;  
  xlatTable[PS2_KEY_F]=0x21;
  xlatTable[PS2_KEY_G]=0x22;
  xlatTable[PS2_KEY_H]=0x23;  
  xlatTable[PS2_KEY_J]=0x24;
  xlatTable[PS2_KEY_K]=0x25;
  xlatTable[PS2_KEY_L]=0x26;
  xlatTable[PS2_KEY_SEMI]=0x27;
  xlatTable[PS2_KEY_APOS]=0x28; 
  xlatTable[PS2_KEY_SINGLE]=0x29;
  xlatTable[PS2_KEY_L_SHIFT]=0x2A;
  xlatTable[PS2_KEY_BACK]=0x2B;
  xlatTable[PS2_KEY_Z]=0x2C;
  xlatTable[PS2_KEY_X]=0x2D;
  xlatTable[PS2_KEY_C]=0x2E;
  xlatTable[PS2_KEY_V]=0x2F;
  xlatTable[PS2_KEY_B]=0x30;
  xlatTable[PS2_KEY_N]=0x31;
  xlatTable[PS2_KEY_M]=0x32;
  xlatTable[PS2_KEY_COMMA]=0x33;
  xlatTable[PS2_KEY_DOT]=0x34;
  xlatTable[PS2_KEY_DIV]=0x35;
  xlatTable[PS2_KEY_KP_DIV]=0x35;
  xlatTable[PS2_KEY_R_SHIFT]=0x36;
  xlatTable[PS2_KEY_KP_TIMES]=0x37;
  xlatTable[PS2_KEY_L_ALT]=0x38;
  xlatTable[PS2_KEY_R_ALT]=0x38;
  xlatTable[PS2_KEY_SPACE]=0x39;
  xlatTable[PS2_KEY_CAPS]=0x3A;
  xlatTable[PS2_KEY_F1]=0x3B;
  xlatTable[PS2_KEY_F2]=0x3C;
  xlatTable[PS2_KEY_F3]=0x3D;
  xlatTable[PS2_KEY_F4]=0x3E;
  xlatTable[PS2_KEY_F5]=0x3F;
  xlatTable[PS2_KEY_F6]=0x40;
  xlatTable[PS2_KEY_F7]=0x41;
  xlatTable[PS2_KEY_F8]=0x42;
  xlatTable[PS2_KEY_F9]=0x43;
  xlatTable[PS2_KEY_F10]=0x44;  
  xlatTable[PS2_KEY_NUM]=0x45;
  xlatTable[PS2_KEY_SCROLL]=0x46;
  xlatTable[PS2_KEY_KP7]=0x47;       // Home or 7
  xlatTable[PS2_KEY_HOME]=0x57;      // CUSTOM (always Home key)
  xlatTable[PS2_KEY_KP8]=0x48;       // Up or 8
  xlatTable[PS2_KEY_UP_ARROW]=0x58;  // CUSTOM (always Up arrow)
  xlatTable[PS2_KEY_KP9]=0x49;       // Pgup or 9
  xlatTable[PS2_KEY_PGUP]=0x59;      // CUSTOM (always PgUp)
  xlatTable[PS2_KEY_KP_MINUS]=0x4A;
  xlatTable[PS2_KEY_KP4]=0x4B;       // Left arrow or 4
  xlatTable[PS2_KEY_L_ARROW]=0x5B;   // CUSTOM (always Left arrow)
  xlatTable[PS2_KEY_KP5]=0x4C;
  xlatTable[PS2_KEY_KP6]=0x4D;       // Right arrow or 6
  xlatTable[PS2_KEY_R_ARROW]=0x5D;   // CUSTOM (always Right arrow)
  xlatTable[PS2_KEY_KP_PLUS]=0x4E;
  xlatTable[PS2_KEY_KP1]=0x4F;       // End or 1
  xlatTable[PS2_KEY_END]=0x5F;       // CUSTOM (always End key)
  xlatTable[PS2_KEY_KP2]=0x50;       // Down arrow or 2
  xlatTable[PS2_KEY_DN_ARROW]=0x60;  // CUSTOM (always Down arrow)
  xlatTable[PS2_KEY_KP3]=0x51;       // Pgdn or 3
  xlatTable[PS2_KEY_PGDN]=0x61;      // CUSTOM (always PgDn)
  xlatTable[PS2_KEY_KP0]=0x52;       // Ins or 0
  xlatTable[PS2_KEY_INSERT]=0x62;    // CUSTOM (always Insert)
  xlatTable[PS2_KEY_KP_DOT]=0x53;    // Delete or dot
  xlatTable[PS2_KEY_DELETE]=0x63;    // CUSTOM (always Delete)
  xlatTable[PS2_KEY_BREAK]=0x64;     // CUSTOM (always Ctrl+Break)
  xlatTable[PS2_KEY_PAUSE]=0x65;     // CUSTOM (always Pause)
  xlatTable[PS2_KEY_PRTSCR]=0x66;    // CUSTOM (always Printscreen)
}
/* ********************************************************************************************** */

// Inlined into setup()
inline void InitializeKeyboard()
{
  // Clear stuff up
  xtKeybClock = xtKeybData = xtKeybAvailable = atKeybLockKeys = 0;
  
  // Using an XT keyboard?
  if (usingXtKeyboard)
  {
    // If the data line is low, there might be 2 start bits...
    if (!((PIND >> 3) & 1))
    {
      xtKeybStartbitsCount = 2;
    }
        
    // Attach ISR to clock line
    attachInterrupt(digitalPinToInterrupt(2), XtKeyboardClockISR, FALLING);
  }
  
  // Using an AT keyboard.
  // Use the PS2Keyboard class, translate scancode into XT
  else
  {
    SetupAtToXtTable();    
    atKeyboard.begin(3, 2);
    
    // Do a soft reset on Arduino reset button...
    atKeyboard.resetKey();
  }
}

// Read an XT scancode from an XT or AT keyboard
uint8_t ReadKeyboard()
{  
  // XT keyboard
  if (usingXtKeyboard)
  {   
    return XtKeyboardRead();
  }
  
  // AT keyboard
  else
  {    
    while(1)
    {
      uint16_t keyIn = atKeyboard.read();
      
      // Translate to XT, reread if not in table
      uint8_t scanCode = xlatTable[(uint8_t)keyIn];
      if (scanCode == 0xff)
      {
        continue;
      }
      
      // PS2KeyAdvanced updates Numlock, Capslock, Scrollock automatically
      // However we disable the keyboard clock immediately after obtaining the scancode
      // so that the keyboard has no time to reflect...
      switch(scanCode)
      {
      case 0x45:
        atKeybLockKeys ^= PS2_LOCK_NUM; break;
      case 0x3A:
        atKeybLockKeys ^= PS2_LOCK_CAPS; break;
      case 0x46:
        atKeybLockKeys ^= PS2_LOCK_SCROLL; break;
      }
            
      // Set MSB if key was released
      if (keyIn & PS2_BREAK)
      {
        scanCode |= 0x80;
      }
      
      return scanCode;
    }
  }
}

// Disables all interrupts and pulls down the data line (XT) or clock (AT)
void DisableKeyboard()
{
  // Clear interrupt flag - this disables the keybclock ISR too
  cli();
  
  // Set appropriate line to output...
  DDRD |= (usingXtKeyboard) ? 8 : 4;
  
  // and pull it down
  PORTD &= (usingXtKeyboard) ? 0xF7 : 0xFB;
}

// The reverse of DisableKeyboard() + with AT/PS2: drop the hung character from buffer
void EnableKeyboard()
{
  // Pullup
  PORTD |= (usingXtKeyboard) ? 8 : 4;
  
  // Set appropriate line to input...
  DDRD &= (usingXtKeyboard) ? 0xF7 : 0xFB;
    
  // Set interrupt flag and re-enable all interrupt handlers
  sei();
  
  // PS2KeyAdvanced: drop the hung character and set proper states
  if (!usingXtKeyboard)
  {
    atKeyboard.resetBuffersStates();
    atKeyboard.setLock(atKeybLockKeys);
  }
}

#endif