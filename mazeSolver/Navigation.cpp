#include "Navigation.h"
#include "Config.h"
#include "Motors.h"
#include "Display.h"
#include "TRSensors.h"
#include <Arduino.h>

void turn(unsigned char dir) {
  // Common turn setup
  SetSpeeds(0, 0);
  delay(30);  // Reduced from 50 for faster response
  
  // Visual feedback of current turn
  clearDisplay();
  setTextSize(3);
  setTextColor();
  setCursor(50, 25);
  printChar(dir);
  updateDisplay();
  
  // Different speeds for learning vs solved mode - with higher base speeds
  int turn_speed = !solved ? LRSpeeds0 : LRSpeeds0 * 1.2;  // Increased multiplier from 1.1 to 1.2
  int turn_delay = !solved ? LRDelay0 : LRDelay0 * 0.8;    // Reduced multiplier from 0.9 to 0.8
  int uturn_speed = !solved ? BSpeeds0 : BSpeeds0 * 1.2;   // Increased multiplier
  int uturn_delay = !solved ? BDelay0 : BDelay0 * 0.8;     // Reduced multiplier
  
  // Special handling for variable-angle turns
  int angle_modifier = 1.0; // Default for 90 degrees
  
  // Check sensor pattern to detect turn angle
  if (dir == 'L' || dir == 'R') {
    // Analyze sensor readings to determine turn angle
    int leftmost = sensorValues[0];
    int rightmost = sensorValues[4];
    int middle_sum = sensorValues[1] + sensorValues[2] + sensorValues[3];
    
    // Detect ~45 degree turn (strong on one side, medium in middle)
    if ((dir == 'L' && leftmost > 800 && middle_sum > 1000) || 
        (dir == 'R' && rightmost > 800 && middle_sum > 1000)) {
      angle_modifier = 0.5; // ~45 degree turn needs less rotation
    }
    // Detect ~60 degree turn
    else if ((dir == 'L' && leftmost > 700 && middle_sum > 800) || 
             (dir == 'R' && rightmost > 700 && middle_sum > 800)) {
      angle_modifier = 0.7; // ~60 degree turn
    }
    // Detect gradual curve (possibly part of a circular section)
    else if (middle_sum > 1500) {
      angle_modifier = 0.3; // Very gentle turn for circular sections
    }
  }
  
  switch(dir) {
    case 'L':
      // Turn left with angle-adjusted parameters and higher speed
      SetSpeeds(-turn_speed, turn_speed);
      delay(turn_delay * angle_modifier);
      break;
    case 'R':
      // Turn right with angle-adjusted parameters and higher speed
      SetSpeeds(turn_speed, -turn_speed);
      delay(turn_delay * angle_modifier);
      break;
    case 'B':
      // U-turn with adjusted parameters and higher speed
      SetSpeeds(uturn_speed, -uturn_speed);
      delay(uturn_delay);
      break;
    case 'S':
      // For straight, just a small adjustment to ensure alignment
      SetSpeeds(90, 90);  // Increased from 70 for faster forward movement
      delay(40);          // Reduced from 50
      break;
  }
  
  // Stop after completing turn - reduced delay
  SetSpeeds(0, 0);
  delay(30);  // Reduced from 50
  
  lasttime = millis();   
}

unsigned char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right) {
  // Implement left-hand-on-the-wall or right-hand-on-the-wall strategy
  if (left) {
    if (found_left)
      return 'L';
    else if (found_straight)
      return 'S';
    else if (found_right)
      return 'R';
    else
      return 'B';
  } else {
    if (found_right)
      return 'R';
    else if (found_straight)
      return 'S';
    else if (found_left)
      return 'L';
    else
      return 'B';
  }
}

void adjustPIDForCurve(int proportional, int *base_speed, int *power_difference) {
  int abs_prop = abs(proportional);
  
  // For very sharp curves, reduce speed significantly
  if (abs_prop > CURVE_SLOWDOWN_THRESHOLD) {
    *base_speed = SHARP_CURVE_SPEED;
  } 
  // For moderate curves, reduce speed moderately
  else if (abs_prop > CURVE_SLOWDOWN_THRESHOLD/2) {
    *base_speed = MILD_CURVE_SPEED;
  }
  // For gentle curves (likely circular sections)
  else if (abs_prop > CURVE_SLOWDOWN_THRESHOLD/3) {
    *base_speed = MILD_CURVE_SPEED + 10; // Slightly faster than moderate curves
  }
  
  // Adjust power difference for different curve types
  if (abs_prop > 500) {
    // Check sensor pattern to detect circular sections
    bool wide_curve_pattern = false;
    int sensor_sum = 0;
    for(int i = 0; i < NUM_SENSORS; i++) {
      sensor_sum += sensorValues[i];
    }
    
    // Circular sections typically activate more sensors with more gradual readings
    if (sensor_sum > 2000 && abs_prop < 800) {
      wide_curve_pattern = true;
    }
    
    if (wide_curve_pattern) {
      // For circular sections, use gentler response
      *power_difference = (*power_difference * 9) / 10; // Less aggressive for circular paths
    } else {
      // For sharper turns
      *power_difference = (*power_difference * 12) / 10; // Increase by 20% for sharper turns
    }
  }
}

void follow_segment() {
  int last_proportional = 0;
  long integral = 0;
  float avg_position = 2000; // Start assuming we're centered
  
  // For average sensor readings
  const int readings_count = 3;
  unsigned int last_positions[readings_count] = {2000, 2000, 2000};
  int reading_index = 0;
  
  // For dynamic speed control
  int base_speed;
  int curve_speed_reduction = 0;
  int max_curve_reduction = 40;

  while(1) {
    // Get the position of the line
    unsigned int raw_position = trs.readLine(sensorValues);
    
    // Simple moving average filter for position
    last_positions[reading_index] = raw_position;
    reading_index = (reading_index + 1) % readings_count;
    
    unsigned long position_sum = 0;
    for(int i = 0; i < readings_count; i++) {
      position_sum += last_positions[i];
    }
    position = position_sum / readings_count;
    
    // Exponential smoothing for more stable position
    avg_position = 0.7 * avg_position + 0.3 * position;
    
    // The "proportional" term should be 0 when we are on the line
    int proportional = ((int)avg_position) - 2000;
    
    // Detect if we're in a curve based on sensor readings
    bool in_curve = false;
    int active_sensors = 0;
    for(int i = 0; i < NUM_SENSORS; i++) {
      if(sensorValues[i] > CURVE_DETECTION_THRESHOLD) {
        active_sensors++;
      }
    }
    
    // Detect a curve by checking pattern and distribution of active sensors
    if(active_sensors >= 2 && abs(proportional) > 500) {
      in_curve = true;
      // Gradually increase curve_speed_reduction up to max_curve_reduction
      if(curve_speed_reduction < max_curve_reduction) {
        curve_speed_reduction += 2;
      }
    } else {
      // Gradually return to normal speed
      if(curve_speed_reduction > 0) {
        curve_speed_reduction--;
      }
    }
    
    // Compute the derivative (change) and integral (sum) of the position
    int derivative = proportional - last_proportional;
    integral += proportional;
    
    // Prevent integral windup by limiting its range
    if(integral > 20000) integral = 20000;
    if(integral < -20000) integral = -20000;
    
    // If we're centered on the line, decay the integral term
    if(abs(proportional) < 100) {
      integral = integral * 0.8;
    }
    
    // Remember the last position
    last_proportional = proportional;
    
    // Compute the difference between the two motor power settings
    int power_difference = (proportional * KP) + (integral * KI) + (derivative * KD);
    
    // Determine base speed - reduce speed in curves
    if(solved) {
      base_speed = 150 - curve_speed_reduction;  // Maximum speed when solved
    } else {
      base_speed = 120 - curve_speed_reduction;  // Learning speed
    }
    
    // Limit the power difference to prevent extreme turns
    int maximum = base_speed;
    if(power_difference > maximum)
      power_difference = maximum;
    if(power_difference < -maximum)
      power_difference = -maximum;
    
    // Apply power difference to motors
    if(power_difference < 0) {
      analogWrite(PWMA, base_speed + power_difference);
      analogWrite(PWMB, base_speed);
    } else {
      analogWrite(PWMA, base_speed);
      analogWrite(PWMB, base_speed - power_difference);
    }
    
    // Check for intersections or dead ends only if enough time has passed
    if(millis() - lasttime > 100) {
      // Check if all sensors moved away from the line (dead end)
      bool all_sensors_off_line = true;
      for(int i = 1; i <= 3; i++) {
        if(sensorValues[i] > LINE_THRESHOLD) {
          all_sensors_off_line = false;
          break;
        }
      }
      
      if(all_sensors_off_line) {
        // No line visible ahead - must be a dead end
        SetSpeeds(0, 0);
        return;
      }
      
      // Improved intersection detection - more sensitive to left side inputs
      if((sensorValues[0] > INTERSECTION_THRESHOLD && (sensorValues[1] > LINE_THRESHOLD || sensorValues[4] > INTERSECTION_THRESHOLD)) || 
         (sensorValues[0] > INTERSECTION_THRESHOLD/1.5 && sensorValues[1] > LINE_THRESHOLD) ||
         (sensorValues[4] > INTERSECTION_THRESHOLD && (sensorValues[3] > LINE_THRESHOLD || sensorValues[0] > INTERSECTION_THRESHOLD))) {
        
        // If left turn potentially detected, slow down to avoid overshooting
        if(sensorValues[0] > INTERSECTION_THRESHOLD/1.5) {
          SetSpeeds(40, 40); // Slow approach to the intersection
          delay(20);
        }
        
        // Found an intersection
        SetSpeeds(0, 0);
        return;
      }
    }
  }
}
