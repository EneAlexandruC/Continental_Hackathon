#ifndef NAVIGATION_H
#define NAVIGATION_H

// Forward declarations
class TRSensors;
extern TRSensors trs;
extern unsigned int sensorValues[];
extern unsigned int position;
extern unsigned long lasttime;
extern unsigned int solved;
extern unsigned int left;

/**
 * Execute a turn in the specified direction
 * @param dir Direction: 'L' for left, 'R' for right, 'B' for back, 'S' for straight
 */
void turn(unsigned char dir);

/**
 * Select which way to turn based on available exits
 */
unsigned char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right);

/**
 * Helper function for PID tuning in the follow_segment function
 */
void adjustPIDForCurve(int proportional, int *base_speed, int *power_difference);

/**
 * Follow a segment of line until intersection or dead end
 */
void follow_segment();

#endif // NAVIGATION_H
