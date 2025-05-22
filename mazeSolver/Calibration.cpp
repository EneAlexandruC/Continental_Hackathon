#include "Calibration.h"
#include "Config.h"
#include "Motors.h"
#include "Display.h"
#include "I2CUtils.h"
#include <Arduino.h>

void calibrateTurns() {
  displayMessage("Calibration Mode", 1, 0, 0);
  clearDisplay();
  setTextSize(1);
  setTextColor();
  setCursor(0, 0);
  printText("Calibration Mode");
  setCursor(0, 10);
  printText("Press button for each test");
  updateDisplay();
  
  // Wait for button press
  waitForButtonPress();
  
  // Test 90-degree left turn
  displayMessage("Testing Left Turn...", 1, 0, 0);
  delay(800);  // Reduced from 1000
  SetSpeeds(-LRSpeeds0, LRSpeeds0);
  delay(LRDelay0);
  SetSpeeds(0, 0);
  
  // Wait for next test
  waitForButtonPress();
  
  // Test 90-degree right turn
  displayMessage("Testing Right Turn...", 1, 0, 0);
  delay(1000);
  SetSpeeds(LRSpeeds0, -LRSpeeds0);
  delay(LRDelay0);
  SetSpeeds(0, 0);
  
  // Wait for next test
  waitForButtonPress();
  
  // Test U-turn
  displayMessage("Testing U-Turn...", 1, 0, 0);
  delay(1000);
  SetSpeeds(BSpeeds0, -BSpeeds0);
  delay(BDelay0);
  SetSpeeds(0, 0);
  
  displayMessage("Calibration Complete", 1, 0, 0);
  delay(2000);
}
