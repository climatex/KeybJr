#ifndef KEYBPCJR_H
#define KEYBPCJR_H

extern uint8_t usingXtKeyboard;

void SendNumericKeypadScancode(uint8_t, uint8_t);

// Send a PCjr scancode over IR and cable.
// The ugliness of this function is proportional to the possibilities of special key combinations.
void SendScancode(uint8_t scanCode)
{
  static uint8_t prevCode = 0;
  static uint8_t numLock = 0;
  static uint8_t shiftState = 0;
  static uint8_t ctrlState = 0;
  static uint8_t altState = 0;
  
  // Cancel out the same MSB flag during checking...
  uint8_t code = scanCode & 0x7f;
  uint8_t wasReleased = (code != scanCode);
            
  // Fast things first - scancodes that do not need to be translated for the Junior.
  // I.e. scancodes under CapsLock index, incl. most alphanumeric keys.
  // Exceptions: accent/tilde, backslash, Printscreen
  if ((code < 0x3a) && (code != 0x29) && (code != 0x2B) && (code != 0x37))
  {        
    // Transmit original scancode
    TransmitByte(scanCode);
    
    // Record ctrl state
    if (code == 0x1D)
    {
      ctrlState = !wasReleased;
    }
    
    // Shift state
    else if (code == 0x2A)
    {
      shiftState = !wasReleased;
    }
    
    // the other shift key, do an OR    
    else if (code == 0x36)
    {
      shiftState |= !wasReleased;
    }
    
    // Alt state
    else if (code == 0x38)
    {
      altState = !wasReleased;
    }
  }
  
  // Reverse apostrophe (grave accent) or tilde
  else if (code == 0x29)
  {
    // That first thang, transmit: Alt + "
    if (!shiftState)
    {
      TransmitByte(wasReleased ? 0xB8 : 0x38);
      TransmitByte(wasReleased ? 0xA8 : 0x28);       
    }
    
    // Tilde, transmit: Alt + right bracket
    else
    {
      TransmitByte(wasReleased ? 0xB8 : 0x38);
      TransmitByte(wasReleased ? 0x9B : 0x1B); 
    }
  }
  
  // Backslash. Transmit: Alt + forward slash
  else if (code == 0x2B)
  {
    TransmitByte(wasReleased ? 0xB8 : 0x38);
    TransmitByte(wasReleased ? 0xB5 : 0x35);
  }
   
  // Caps Lock. Transmit: Caps Lock
  else if (code == 0x3A)
  { 
    // AT: sends CapsLock keyrelease on the second push
    if (!usingXtKeyboard)
    {
      TransmitByte(0x3A); 
      TransmitByte(0xBA); 
    }
    
    // XT: sends keyrelease immediately
    else
    {
      TransmitByte(scanCode);
    }    
  }
  
  // Scroll Lock (alone, no Ctrl+). Transmit: Fn key + S
  else if ((code == 0x46) && !ctrlState)
  { 
    // AT: sends ScrollLock keyrelease on the second push
    if (!usingXtKeyboard)
    {
      TransmitByte(0x54);
      TransmitByte(0x1F);
      TransmitByte(0x9F);
      TransmitByte(0xD4);
    }
    
    // XT sends key release immediately
    else
    {
      TransmitByte(wasReleased ? 0xD4 : 0x54);
      TransmitByte(wasReleased ? 0x9F : 0x1F);  
    }
  }
  
  // Num Lock (alone, no Ctrl+). Trigger internal flag. Do not send anything
  else if ((code == 0x45) && !ctrlState)
  {    
    // AT numlock sends 0x45 on and 0x45|0x80 (keyrelease) off, without keyrepeat
    if (!usingXtKeyboard)
    {
      numLock ^= 1;
    }
    
    // XT numlock behaves like a regular key, with keyrepeat
    else
    {
      // Flip the numlock only when key pressed and not held
      if ((scanCode == 0x45) && (prevCode != 0x45))
      {
        numLock ^= 1;
      }
    }
  }
  
  // Process keypad based on numlock status.
  else if ((code >= 0x47) && (code <= 0x53))
  {
    // Navigation keys in >100-key keyboards in XT emulation mode with num lock on?
    if (usingXtKeyboard && numLock && shiftState)
    {
      // These usually send a shift state...
      // Cancel it out in the output, temporarily, and transmit it as navigation key
      TransmitByte(0xAA);
      SendNumericKeypadScancode(scanCode, 0);
      TransmitByte(0x2A);
    }
    
    // Normal behavior. Allow Alt+extended codes with both numlock on and off
    else
    {
      SendNumericKeypadScancode(scanCode, !altState && numLock);   
    }
  }
  
  // AT/PS2 navigation keys with clear distinction (cursors, Insert, Delete, Home, End, Pgup, Pgdwn).
  // Here as regular scancode + 0x10. Do process keypad, just like with numlock off
  else if ((code >= 0x57) && (code <= 0x63))
  {
    SendNumericKeypadScancode(scanCode - 0x10, 0);
  }
   
  // F1 to F10 (sorry F11, F12). Transmit: Fn key + 1 to 0
  else if ((code >= 0x3b) && (code <= 0x44))
  {   
    // Fn keypress or release
    TransmitByte(wasReleased ? 0xD4 : 0x54);
    
    // Distance between 1 and F1
    if (!wasReleased)
    {
      uint8_t diff = scanCode - 0x39;
      TransmitByte(diff);
      TransmitByte(diff | 0x80); //create 1 to 0 keyrelease...
    }
  }
  
  // XT: * PrtSc (alone). Transmit: Alt + dot
  else if (usingXtKeyboard && (code == 0x37) && !ctrlState && !shiftState)
  {
    TransmitByte(wasReleased ? 0xB8 : 0x38);
    TransmitByte(wasReleased ? 0xB4 : 0x34);
  }
  
  // Ctrl+Scroll Lock (AT: Control Break). Transmit: Fn key + B
  else if ((code == 0x64) || (ctrlState && (code == 0x46)))
  {
    // On single keypress: Fn, B, release B, release Fn
    // AT: sends keyreleased on second scrolllock press...
    if (!wasReleased || (!usingXtKeyboard && (code != 0x64) /* can do Ctrl+Break on AT*/))
    {
      TransmitByte(0x54);
      TransmitByte(0x30);
      TransmitByte(0xB0);
      TransmitByte(0xD4);
    }
  }
  
  // Ctrl+Print Screen (Echo Print). Transmit: Fn key + E
  else if (ctrlState && ((code == 0x66) || (code == 0x37)))
  {
    // On single keypress: Fn, E, release E, release Fn
    if (!wasReleased)
    {
      TransmitByte(0x54);
      TransmitByte(0x12);
      TransmitByte(0x92);
      TransmitByte(0xD4);
    }
  }
  
  // Shift+Print Screen (AT: Print Screen). Transmit: Fn key + P
  else if ((code == 0x66) || (shiftState && (code == 0x37)))
  {
    // On single keypress: Fn, P, release P, release Fn
    if (!wasReleased)
    {
      TransmitByte(0x54);
      TransmitByte(0x19);
      TransmitByte(0x99);
      TransmitByte(0xD4);
    }
  }
  
  // Ctrl+Num Lock (AT: Pause). Transmit: Fn key + Q
  else if ((code == 0x65) || (ctrlState && (code == 0x45)))
  {
    // On single keypress: Fn, Q, release Q, release Fn
    // AT: sends keyreleased on second numlock press...
    if (!wasReleased || (!usingXtKeyboard && (code != 0x65) /* can do Pause on AT*/))
    {
      TransmitByte(0x54);
      TransmitByte(0x10);
      TransmitByte(0x90);
      TransmitByte(0xD4);
    }
  }
  
  // Pass-through for the rest
  else
  {
    TransmitByte(scanCode);
  }
  
  // Used to detect if a key is being held and repeated (here XT Numlock during toggle)
  prevCode = scanCode;  
}

// Second half of the butt-fugly function is the XT distinction between navigation and numpad...
void SendNumericKeypadScancode(uint8_t scanCode, uint8_t numLock)
{
  // Cancel out the same MSB flag during checking
  uint8_t code = scanCode & 0x7f;
  uint8_t wasReleased = (code != scanCode);
  
  // Home / numpad 7.
  if (code == 0x47)
  {
    // Home, transmit: Fn key + up cursor
    if (!numLock)
    {
      TransmitByte(wasReleased ? 0xD4 : 0x54);
      TransmitByte(wasReleased ? 0xC8 : 0x48);
    }
    
    // Seven. Transmit scancode for 7    
    else
    {
      TransmitByte(wasReleased ? 0x88 : 0x08);
    }
  }
  
  // Cursor up / numpad 8.
  else if (code == 0x48)
  {
    // Cursor up, transmit: up cursor
    if (!numLock)
    {
      TransmitByte(scanCode);
    }
    
    // Eight. Transmit scancode for 8
    else
    {
      TransmitByte(wasReleased ? 0x89 : 0x09);
    }    
  }
  
  // Page Up / numpad 9.
  else if (code == 0x49)
  {
    // Page Up, transmit: Fn key + left cursor
    if (!numLock)
    {
      TransmitByte(wasReleased ? 0xD4 : 0x54);
      TransmitByte(wasReleased ? 0xCB : 0x4B);
    }
    
    // Nine. Transmit scancode for 9
    else
    {
      TransmitByte(wasReleased ? 0x8A : 0x0A);
    }    
  }
  
  // Numpad minus
  else if (code == 0x4A)
  {
    // Transmit: Fn key + minus
    TransmitByte(wasReleased ? 0xD4 : 0x54);
    TransmitByte(wasReleased ? 0x8C : 0x0C); 
  }
  
  // Cursor left / numpad 4.
  else if (code == 0x4B)
  {
    // Cursor left, transmit: left cursor
    if (!numLock)
    {
      TransmitByte(scanCode);
    }
    
    // Four. Transmit scancode for 4
    else
    {
      TransmitByte(wasReleased ? 0x85 : 0x05);
    }    
  }
  
  // Numpad 5, send five only when numlock is on
  else if (code == 0x4C)
  {
    // Five. Transmit scancode for 5
    if (numLock)
    {
      TransmitByte(wasReleased ? 0x86 : 0x06);
    }
  }
  
  // Cursor right / numpad 6.
  else if (code == 0x4D)
  {
    // Cursor right, transmit: right cursor
    if (!numLock)
    {
      TransmitByte(scanCode);
    }
    
    // Six. Transmit scancode for 6
    else
    {
      TransmitByte(wasReleased ? 0x87 : 0x07);
    }    
  }
  
  // Numpad plus
  else if (code == 0x4E)
  {
    // Transmit: Fn key + plus
    TransmitByte(wasReleased ? 0xD4 : 0x54);
    TransmitByte(wasReleased ? 0x8D : 0x0D); 
  }
  
  // End / numpad 1.
  else if (code == 0x4F)
  {
    // End, transmit: Fn key + down cursor
    if (!numLock)
    {
      TransmitByte(wasReleased ? 0xD4 : 0x54);
      TransmitByte(wasReleased ? 0xD0 : 0x50);  
    }
    
    // One. Transmit scancode for 1
    else
    {
      TransmitByte(wasReleased ? 0x82 : 0x02);
    }    
  }
  
  // Cursor down / numpad 2.
  else if (code == 0x50)
  {
    // Cursor down, transmit: down cursor
    if (!numLock)
    {
      TransmitByte(scanCode);
    }
    
    // Two. Transmit scancode for 2
    else
    {
      TransmitByte(wasReleased ? 0x83 : 0x03);
    }    
  }
  
  // Page Down / numpad 3.
  else if (code == 0x51)
  {
    // Page Down, transmit: Fn key + right cursor
    if (!numLock)
    {
      TransmitByte(wasReleased ? 0xD4 : 0x54);
      TransmitByte(wasReleased ? 0xCD : 0x4D);  
    }
    
    // Three. Transmit scancode for 3
    else
    {
      TransmitByte(wasReleased ? 0x84 : 0x04);
    }
  }
  
  // Ins / numpad 0.
  else if (code == 0x52)
  {
    // Transmit: insert
    if (!numLock)
    {
      TransmitByte(scanCode);
    }
    
    // Zero. Transmit scancode for 0
    else
    {
      TransmitByte(wasReleased ? 0x8B : 0x0B);
    }
  }
  
  // Del / numpad dot
  else if (code == 0x53)
  {
    // Transmit: delete
    if (!numLock)
    {
      TransmitByte(scanCode);
    }
    
    // Numpad dot. Transmit: Shift + Del
    else
    {
      TransmitByte(wasReleased ? 0xAA : 0x2A);      
      TransmitByte(scanCode);
    }
  } 
}

#endif