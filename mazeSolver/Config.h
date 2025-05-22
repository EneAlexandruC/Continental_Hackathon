#ifndef CONFIG_H
#define CONFIG_H

// Pin definitions
#define PWMA   6           // Left Motor Speed pin (ENA)
#define AIN2   A0          // Motor-L forward (IN2)
#define AIN1   A1          // Motor-L backward (IN1)
#define PWMB   5           // Right Motor Speed pin (ENB)
#define BIN1   A2          // Motor-R forward (IN3)
#define BIN2   A3          // Motor-R backward (IN4)
#define PIN 7              // RGB LED pin
#define NUM_SENSORS 5
#define OLED_RESET 9
#define OLED_SA0   8
#define Addr  0x20

// Turn parameters
#define LRSpeeds0 130        // Speed for left/right turns (increased from 100)
#define LRDelay0 230         // Delay for left/right turns (reduced from 290)
#define BSpeeds0 180         // Speed for u-turns (increased from 150)
#define BDelay0 300          // Delay for u-turns (reduced from 350)

// Curve handling parameters
#define MILD_CURVE_SPEED 120  // Increased from 100
#define SHARP_CURVE_SPEED 90  // Increased from 80
#define CURVE_SLOWDOWN_THRESHOLD 800

// PID constants
#define KP 0.25     // Proportional constant
#define KI 0.0001   // Integral constant
#define KD 2.0      // Derivative constant

// Line detection thresholds
#define LINE_THRESHOLD 300
#define INTERSECTION_THRESHOLD 500
#define CURVE_DETECTION_THRESHOLD 400

#endif // CONFIG_H
