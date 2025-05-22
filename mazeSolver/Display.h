#ifndef DISPLAY_H
#define DISPLAY_H

// We'll use wrapper functions instead of exposing display directly
// to avoid incomplete type issues

/**
 * Initialize the display
 */
void initDisplay();

/**
 * Display a message on the OLED screen
 */
void displayMessage(const char* message, int size, int x, int y);

/**
 * Display a multi-line message
 */
void displayMultipleLines(const char* line1, const char* line2, int size1, int size2);

/**
 * Clear the display
 */
void clearDisplay();

/**
 * Set text size
 */
void setTextSize(int size);

/**
 * Set text color
 */
void setTextColor();

/**
 * Set cursor position
 */
void setCursor(int x, int y);

/**
 * Print text at current cursor position
 */
void printText(const char* text);

/**
 * Print a single character at current cursor position
 */
void printChar(char c);

/**
 * Update the display with all pending changes
 */
void updateDisplay();

/**
 * Draw a line of underscores
 */
void drawUnderscoreLine(int count);

#endif // DISPLAY_H
