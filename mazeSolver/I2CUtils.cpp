#include "I2CUtils.h"
#include "Config.h"
#include <Wire.h>

void PCF8574Write(byte data) {
  Wire.beginTransmission(Addr);
  Wire.write(data);
  Wire.endTransmission(); 
}

byte PCF8574Read() {
  int data = -1;
  Wire.requestFrom(Addr, 1);
  if(Wire.available()) {
    data = Wire.read();
  }
  return data;
}

bool waitForButtonPress(unsigned long timeout) {
  byte buttonValue = 0;
  unsigned long startTime = millis();
  
  while(buttonValue != 0xEF) {
    PCF8574Write(0x1F | PCF8574Read());
    buttonValue = PCF8574Read() | 0xE0;
    
    if(timeout > 0 && (millis() - startTime > timeout)) {
      return false;
    }
    delay(5);  // Reduced from 10 for faster button response
  }
  return true;
}
