#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

/*
 * Read byte from EEPROM
 */
byte er_byte(int addr) {
#if DebugReadWrite == true
  Serial.print (F("R-> "));
  Serial.print (addr);
  Serial.print (F(" byte: "));
  Serial.println (EEPROM.read(addr));
#endif

  return EEPROM.read(addr);
}

/*
 * Update byte in EEPROM
 */
void ew_byte(int addr, byte data) {
#if DebugReadWrite == true
  byte old = EEPROM.read(addr);
  Serial.print (F("U-> "));
  Serial.print (addr);
  Serial.print (F(" byte: "));
  Serial.print (old);
  Serial.print (F(" => "));
  Serial.println (data);
  if (data != old)
    EEPROM.write(addr, data);
#else
  if (data != EEPROM.read(addr))
    EEPROM.write(addr, data);
#endif
}


#endif
