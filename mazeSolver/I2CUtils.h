#ifndef I2CUTILS_H
#define I2CUTILS_H

#include <Arduino.h>

/**
 * Write data to PCF8574 I/O expander
 */
void PCF8574Write(byte data);

/**
 * Read data from PCF8574 I/O expander
 */
byte PCF8574Read();

/**
 * Wait for a button press with optional timeout
 * @param timeout Timeout in milliseconds (0 = wait forever)
 * @return True if button was pressed, false if timeout occurred
 */
bool waitForButtonPress(unsigned long timeout = 0);

#endif // I2CUTILS_H
