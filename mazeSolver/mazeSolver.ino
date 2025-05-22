#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "EEPROM.h"
#include "TRSensors.h"

#include "Config.h"
#include "Motors.h"
#include "Display.h"
#include "I2CUtils.h"
#include "Navigation.h"
#include "PathManagement.h"
#include "Calibration.h"

// Global variables - moved display to Display.cpp
TRSensors trs = TRSensors();
Adafruit_NeoPixel RGB = Adafruit_NeoPixel(4, PIN, NEO_GRB + NEO_KHZ800);

unsigned int sensorValues[NUM_SENSORS];
unsigned int position;
uint16_t i, j;
byte value;
unsigned long lasttime = 0;
unsigned int solved = 0;
unsigned int left = 1;

char path[100] = "";
unsigned char path_length = 0;
unsigned char times_length = 0;

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("TRSensor example");
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize motor control pins
  pinMode(PWMA, OUTPUT);                     
  pinMode(AIN2, OUTPUT);      
  pinMode(AIN1, OUTPUT);
  pinMode(PWMB, OUTPUT);       
  pinMode(AIN1, OUTPUT);     
  pinMode(AIN2, OUTPUT);  
  SetSpeeds(0, 0);
  
  // Initialize display
  initDisplay();
  displayMultipleLines("CONTINENTAL", "HACKATHON", 1, 1);
  setTextSize(1);
  setCursor(5, 55);
  printText("Apasa pentru calibrare");
  updateDisplay();
  
  int t0 = millis();
  
  // Choose maze solving strategy
  displayMessage("STANGA", 1, 0, 0);
  
  while(value != 0xEF) {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;

    if (millis() - t0 > 3000 && millis() - t0 < 9999) {
      displayMessage("DREAPTA", 1, 0, 0);
      left = 0;
    }

    if (millis() - t0 > 10000) {
      displayMessage("Invatat", 1, 0, 0);
      left = 7;
    }
  }

  // If previously solved, load the path
  if(left == 7) {
    solved = 1;
    loadPath();
  }
  
  // Initialize RGB LEDs
  RGB.begin();
  RGB.setPixelColor(0, 0x020F0);
  RGB.setPixelColor(1, 0x020F0);
  RGB.setPixelColor(2, 0x020F0);
  RGB.setPixelColor(3, 0x020F0);
  RGB.show(); 
  delay(500);
  
  // Sensor calibration routine
  for (int i = 0; i < 100; i++) {
    if(i < 25 || i >= 75) {
      digitalWrite(AIN2, HIGH);
      digitalWrite(AIN1, LOW);
      digitalWrite(BIN1, LOW); 
      digitalWrite(BIN2, HIGH);  
      SetSpeeds(70, -70);
    } else {
      digitalWrite(AIN2, LOW);
      digitalWrite(AIN1, HIGH);
      digitalWrite(BIN1, HIGH); 
      digitalWrite(BIN2, LOW);  
      SetSpeeds(-70, 70);
    }
    trs.calibrate();
  }
  
  SetSpeeds(0, 0); 
  
  // Set LEDs to white to indicate calibration complete
  RGB.setPixelColor(0, 0xF0F0F0);
  RGB.setPixelColor(1, 0xF0F0F0);
  RGB.setPixelColor(2, 0xF0F0F0);
  RGB.setPixelColor(3, 0xF0F0F0);
  RGB.show();
  
  // Wait for button press to start
  value = 0;
  while(value != 0xEF) {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
    position = trs.readLine(sensorValues)/200;
    clearDisplay();
    setCursor(0, 25);
    printText("Calibrare reusita !!!");
    setCursor(0, 55);
    drawUnderscoreLine(21);
    setCursor(position*6, 55);
    printText("**");
    updateDisplay();
  }

  displayMessage("Acum invat!", 1, 10, 25);
  delay(500);
}

void loop() {
  // Learning mode - find the solution
  if(left != 7) {
    while(1) {
      follow_segment();

      // Drive straight to center in the intersection - faster
      SetSpeeds(50, 50);  // Increased from 30
      delay(30);          // Reduced from 40

      // Check intersection type
      unsigned char found_left = 0;
      unsigned char found_straight = 0;
      unsigned char found_right = 0;

      trs.readLine(sensorValues);

      // Check for left and right exits
      if (sensorValues[0] > 500)
        found_left = 1;
      if (sensorValues[4] > 500)
        found_right = 1;

      // Drive straight a bit more for better alignment - faster
      SetSpeeds(70, 70);  // Increased from 50
      if (!solved) {
        delay(150);       // Reduced from 200
      } else {
        delay(100);       // Reduced from 150
      }
      
      SetSpeeds(0, 0);
      delay(30);          // Reduced from 50

      // Check for a straight exit
      trs.readLine(sensorValues);
      if (sensorValues[1] > 500 || sensorValues[2] > 500 || sensorValues[3] > 500)
        found_straight = 1;

      // Check if we've reached the end
      if (sensorValues[0] > 500 && sensorValues[1] > 500 && sensorValues[2] > 500 && 
          sensorValues[3] > 500 && sensorValues[4] > 500) {
        displayMessage("Am gasit iesirea!", 2, 0, 25);
        savePath();
        solved = 1;
        SetSpeeds(0, 0);
        break;
      }

      // Select turn direction and make the turn
      unsigned char dir = select_turn(found_left, found_straight, found_right);
      turn(dir);

      // Store the turn in the path
      path[path_length] = dir;
      path_length++;

      // Simplify the path when possible
      simplify_path();
    }
  }

  // Solved mode - rerun the maze using the stored solution
  while(1) {
    // Wait for button press to start rerun
    SetSpeeds(0, 0);
    Serial.println("End !!!");
    
    value = 0;
    while(value != 0xEF) {
      PCF8574Write(0x1F | PCF8574Read());
      value = PCF8574Read() | 0xE0;
    }
    delay(1000);

    // Follow the stored path
    for (int i = 0; i < path_length; i++) {
      follow_segment();
      SetSpeeds(30, 30);
      delay(100);
      turn(path[i]);
    }

    // Follow the last segment to the finish
    follow_segment();
  }
}