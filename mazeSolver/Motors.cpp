#include "Config.h"
#include "Motors.h"
#include <Arduino.h>

void SetSpeeds(int Aspeed, int Bspeed) {
  // Control left motor
  if(Aspeed < 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, -Aspeed);      
  } else {
    digitalWrite(AIN1, LOW); 
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, Aspeed);  
  }
  
  // Control right motor
  if(Bspeed < 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, -Bspeed);      
  } else {
    digitalWrite(BIN1, LOW); 
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, Bspeed);  
  }
}
