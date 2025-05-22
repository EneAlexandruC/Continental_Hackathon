#include "Display.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

// The display instance is defined here to avoid including Adafruit_SSD1306.h elsewhere
Adafruit_SSD1306 display(OLED_RESET, OLED_SA0);

void initDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void displayMessage(const char* message, int size, int x, int y) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(message);
  display.display();
}

void displayMultipleLines(const char* line1, const char* line2, int size1, int size2) {
  display.clearDisplay();
  display.setTextSize(size1);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.println(line1);
  display.setCursor(10, 25);
  display.setTextSize(size2);
  display.println(line2);
  display.display();
}

void clearDisplay() {
  display.clearDisplay();
}

void setTextSize(int size) {
  display.setTextSize(size);
}

void setTextColor() {
  display.setTextColor(WHITE);
}

void setCursor(int x, int y) {
  display.setCursor(x, y);
}

void printText(const char* text) {
  display.println(text);
}

void printChar(char c) {
  display.println(c);
}

void updateDisplay() {
  display.display();
}

void drawUnderscoreLine(int count) {
  for (int i = 0; i < count; i++) {
    display.print('_');
  }
}
