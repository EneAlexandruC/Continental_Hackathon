#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "TRSensors.h"
#include <Wire.h>
#include "EEPROM.h"

// Updated turning parameters - for smooth, controlled turns
#define LRSpeeds0 150        // Higher for faster, but still controlled turns
#define LRDelay0 150         // Shorter for snappier turns
#define BSpeeds0 180         // Higher for faster U-turns
#define BDelay0 240          // Shorter for faster U-turns

// Curve handling parameters
#define MILD_CURVE_SPEED 180  // Higher for faster curves
#define SHARP_CURVE_SPEED 150 // Higher for sharp curves
#define CURVE_SLOWDOWN_THRESHOLD 1000  // Higher for later slowdown in curves

#define PWMA   6           //Left Motor Speed pin (ENA)
#define AIN2   A0          //Motor-L forward (IN2).
#define AIN1   A1          //Motor-L backward (IN1)
#define PWMB   5           //Right Motor Speed pin (ENB)
#define BIN1   A2          //Motor-R forward (IN3)
#define BIN2   A3          //Motor-R backward (IN4)
#define PIN 7
#define NUM_SENSORS 5
#define OLED_RESET 9
#define OLED_SA0   8
#define Addr  0x20

Adafruit_SSD1306 display(OLED_RESET, OLED_SA0);

TRSensors trs =   TRSensors();
unsigned int sensorValues[NUM_SENSORS];
unsigned int position;
uint16_t i, j;
byte value;
unsigned long lasttime = 0;
Adafruit_NeoPixel RGB = Adafruit_NeoPixel(4, PIN, NEO_GRB + NEO_KHZ800);
unsigned int solved = 0;
unsigned int left = 1;

void PCF8574Write(byte data);
byte PCF8574Read();
uint32_t Wheel(byte WheelPos);

char path[100] = "";
unsigned char path_length = 0; // the length of the path


void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("TRSensor example");
  Wire.begin();
  pinMode(PWMA,OUTPUT);                     
  pinMode(AIN2,OUTPUT);      
  pinMode(AIN1,OUTPUT);
  pinMode(PWMB,OUTPUT);       
  pinMode(AIN1,OUTPUT);     
  pinMode(AIN2,OUTPUT);  
  SetSpeeds(0,0);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the spxdd d lashscreen.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.println("CONTINENTAL");
  display.setCursor(10,25);
  display.setTextSize(1);
  display.println("HACKATHON");
  display.setTextSize(1);
  display.setCursor(5,55);
  display.println("Apasa pentru calibrare");
  display.display();
  
  int t0 = millis();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("STANGA");
  display.display();


  while(value != 0xEF)  //wait button pressed
  {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;

    if (millis() - t0 > 3000 && millis() - t0 <9999)
    {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("DREAPTA");
      display.display();
      left = 0;
    }

    if (millis() - t0 > 10000)
    {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Invatat");
      display.display();
      left = 7;
    }
  }

  if(left == 7){
    solved =1;
    byte haha = EEPROM.read(0);
    path_length = char(haha);
    for (int i = 1; i <= path_length ; i++) {
        byte readValue = EEPROM.read(i);

        if (readValue == 0) {
            break;
        }
        char readValueChar = char(readValue);
        path[i-1]=readValueChar ;
    }
  }
  
  RGB.begin();
  RGB.setPixelColor(0,0x020F0 );
  RGB.setPixelColor(1,0x020F0 );
  RGB.setPixelColor(2,0x020F0 );
  RGB.setPixelColor(3,0x020F0 );
  RGB.show(); 
  delay(500);
//  analogWrite(PWMA,60);
//  analogWrite(PWMB,60);
  for (int i = 0; i < 100; i++)  // make the calibration take about 10 seconds
  {
    if(i < 25 || i >= 75)
    {
     digitalWrite(AIN2,HIGH);
     digitalWrite(AIN1,LOW);
     digitalWrite(BIN1,LOW); 
     digitalWrite(BIN2,HIGH);  
      SetSpeeds(70,-70);
    }
    else
    {
     digitalWrite(AIN2,LOW);
     digitalWrite(AIN1,HIGH);
     digitalWrite(BIN1,HIGH); 
     digitalWrite(BIN2,LOW);  
        SetSpeeds(-70,70);
    }
    trs.calibrate();       // reads all sensors 100 times
  }
  SetSpeeds(0,0); 
  RGB.setPixelColor(0,0xF0F0F0 );
  RGB.setPixelColor(1,0xF0F0F0 );
  RGB.setPixelColor(2,0xF0F0F0 );
  RGB.setPixelColor(3,0xF0F0F0 );
  RGB.show(); // Initialize all pixels to 'off'
  
  value = 0;
  while(value != 0xEF)  //wait button pressed
  {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
    position = trs.readLine(sensorValues)/200;
    display.clearDisplay();
    display.setCursor(0,25);
    display.println("Calibrare reusita !!!");
    display.setCursor(0,55);
    for (int i = 0; i < 21; i++)
    {
      display.print('_');
    }
    display.setCursor(position*6,55);
    display.print("**");
    display.display();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,25);
  display.println("Acum invat!");
  display.display();
  delay(500);
}

// PID constants - tuned for fast, stable line following
#define KP 0.28   // More aggressive for fast correction
#define KI 0.00012 // Slightly higher for quick integral response
#define KD 2.7    // Higher for stronger derivative damping

// Threshold values for sensors
#define LINE_THRESHOLD 300      // Minimum value to consider as line
#define INTERSECTION_THRESHOLD 420 // Lowered for more sensitive intersection detection
#define CURVE_DETECTION_THRESHOLD 400 // Threshold to detect curve

// Increase base speeds for faster movement
#define MILD_CURVE_SPEED 130  // Increased from 100
#define SHARP_CURVE_SPEED 110 // Increased from 80
#define CURVE_SLOWDOWN_THRESHOLD 900  // Slightly higher for faster response

void follow_segment()
{
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

  while(1)
  {
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
    
    // Only learning mode, so always use max speed
    base_speed = 180 - curve_speed_reduction;  // Slightly higher for more speed
    
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
    // This prevents multiple triggers when approaching intersections
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
      
      // Improved intersection detection
      // Only trigger on outer sensors when they're significantly above threshold
      // and we're not in a gradual curve (which would activate only one outer sensor)
      if((sensorValues[0] > INTERSECTION_THRESHOLD && (sensorValues[1] > LINE_THRESHOLD || sensorValues[4] > INTERSECTION_THRESHOLD)) || 
         (sensorValues[4] > INTERSECTION_THRESHOLD && (sensorValues[3] > LINE_THRESHOLD || sensorValues[0] > INTERSECTION_THRESHOLD))) {
        // Found an intersection
        SetSpeeds(0, 0);
        return;
      }
    }
  }
}

// Improved turn function for AlphaBot2-AR: single, smooth in-place turn for sharp turns
void turn(unsigned char dir)
{
  // Brief stop before turning for stability
  SetSpeeds(0, 0);
  delay(30);

  // Visual feedback of current turn
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(50,25);
  display.println((char)dir);
  display.display();

  // For AlphaBot2-AR: use strong in-place turns for 90°/U-turns, gentle curves for 'S'
  int turn_speed = LRSpeeds0; // Use smooth speed
  int turn_delay = LRDelay0;
  int uturn_speed = BSpeeds0;
  int uturn_delay = BDelay0;

  switch(dir)
  {
    case 'L':
      // Single, strong in-place left turn
      SetSpeeds(-turn_speed, turn_speed);
      delay(turn_delay + 30); // Slightly longer for AlphaBot2-AR
      break;
    case 'R':
      // Single, strong in-place right turn
      SetSpeeds(turn_speed, -turn_speed);
      delay(turn_delay + 30);
      break;
    case 'B':
      // U-turn in place
      SetSpeeds(uturn_speed, -uturn_speed);
      delay(uturn_delay + 50);
      break;
    case 'S':
      // For straight, just a small forward nudge for alignment
      SetSpeeds(turn_speed, turn_speed);
      delay(40);
      break;
  }

  // Stop after completing turn
  SetSpeeds(0, 0);
  delay(30);

  lasttime = millis();   
}

// Helper function for PID tuning in the follow_segment function
void adjustPIDForCurve(int proportional, int *base_speed, int *power_difference) {
  // Adjust speed based on how sharp the curve is (indicated by proportional)
  int abs_prop = abs(proportional);
  
  // For very sharp curves, reduce speed significantly
  if (abs_prop > CURVE_SLOWDOWN_THRESHOLD) {
    *base_speed = SHARP_CURVE_SPEED;
  } 
  // For moderate curves, reduce speed moderately
  else if (abs_prop > CURVE_SLOWDOWN_THRESHOLD/2) {
    *base_speed = MILD_CURVE_SPEED;
  }
  
  // Adjust power difference for sharper response in curves
  if (abs_prop > 500) {
    *power_difference = (*power_difference * 12) / 10; // Increase by 20% for sharper turns
  }
}

unsigned char times_length = 0;

// The path variable will store the path that the robot has taken.  It
// is stored as an array of characters, each of which represents the
// turn that should be made at one intersection in the sequence:
//  'L' for left
//  'R' for right
//  'S' for straight (going straight through an intersection)
//  'B' for back (U-turn)
//
// Whenever the robot makes a U-turn, the path can be simplified by
// removing the dead end.  The follow_next_turn() function checks for
// this case every time it makes a turn, and it simplifies the path
// appropriately.

// This function decides which way to turn during the learning phase of
// maze solving.  It uses the variables found_left, found_straight, and
// found_right, which indicate whether there is an exit in each of the
// three directions, applying the "left hand on the wall" strategy.
unsigned char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right)
{
  // Make a decision about how to turn.  The following code
  // implements a left-hand-on-the-wall strategy, where we always
  // turn as far to the left as possible.
  if (left)
  {
    if (found_left)
      return 'L';
    else if (found_straight)
      return 'S';
    else if (found_right)
      return 'R';
    else
      return 'B';
  }
  else
  {
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

// Path simplification.  The strategy is that whenever we encounter a
// sequence xBx, we can simplify it by cutting out the dead end.  For
// example, LBL -> S, because a single S bypasses the dead end
// represented by LBL.
void simplify_path()
{
  // only simplify the path if the second-to-last turn was a 'B'
  if (path_length < 3 || path[path_length-2] != 'B')
    return;

  int total_angle = 0;
  int i;

  if (left)
  {
    for (i = 1; i <= 3; i++)
    {
      switch (path[path_length - i])
      {
      case 'R':
        total_angle += 90;
        break;
      case 'L':
        total_angle += 270;
        break;
      case 'B':
        total_angle += 180;
        break;
      }
    }
  }else
  {
    for (i = 1; i <= 3; i++)
    {
      switch (path[path_length - i])
      {
      case 'L':
        total_angle += 90;
        break;
      case 'R':
        total_angle += 270;
        break;
      case 'B':
        total_angle += 180;
        break;
      }
    }
  }

  // Get the angle as a number between 0 and 360 degrees.
  total_angle = total_angle % 360;

  // Replace all of those turns with a single one.
  switch (total_angle)
  {
  case 0:
    path[path_length - 3] = 'S';
    break;
  case 90:
    path[path_length - 3] = left == 0 ? 'L' : 'R';
    break;
  case 180:
    path[path_length - 3] = 'B';
    break;
  case 270:
    path[path_length - 3] = left == 0 ? 'R' : 'L';
    break;
  }

  // The path is now two steps shorter.
  path_length -= 2;
}

void loop() {
  if(left!=7)
  while (1)
  {
    follow_segment();

    // Drive straight a bit.  This helps us in case we entered the
    // intersection at an angle.
    // Note that we are slowing down - this prevents the robot
    // from tipping forward too much.
    SetSpeeds(40, 40);
    delay(30);

    unsigned char found_left = 0;
    unsigned char found_straight = 0;
    unsigned char found_right = 0;

    // First intersection check (before creeping forward)
    trs.readLine(sensorValues);
    if (sensorValues[0] > INTERSECTION_THRESHOLD)
      found_left = 1;
    if (sensorValues[4] > INTERSECTION_THRESHOLD)
      found_right = 1;

    // Creep forward into the intersection for better detection
    SetSpeeds(60,60);
    delay(60);
    SetSpeeds(0, 0);
    delay(20);

    // Second intersection check (after creeping forward)
    trs.readLine(sensorValues);
    if (sensorValues[0] > INTERSECTION_THRESHOLD)
      found_left = 1;
    if (sensorValues[4] > INTERSECTION_THRESHOLD)
      found_right = 1;
    if (sensorValues[1] > INTERSECTION_THRESHOLD || sensorValues[2] > INTERSECTION_THRESHOLD || sensorValues[3] > INTERSECTION_THRESHOLD)
      found_straight = 1;

    // Check for the ending spot.
    if (sensorValues[0] > 500 && sensorValues[1] > 500 && sensorValues[2] > 500 && sensorValues[3] > 500 && sensorValues[4] > 500)
    {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,25);
      display.println("Am gasit iesirea!");
      display.display();
      EEPROM.write(0, path_length);
      for (int i = 1; i <= path_length; i++) {
          EEPROM.write(i,path[i-1]);
      }
      solved = 1;
      SetSpeeds(0, 0);
      break;
    }

    // Intersection identification is complete.
    // If the maze has been solved, we can follow the existing
    // path.  Otherwise, we need to learn the solution.
    unsigned char dir = select_turn(found_left, found_straight, found_right);

    // Make the turn indicated by the path.
    turn(dir);

    // Store the intersection in the path variable.
    path[path_length] = dir;
    path_length++;

    // You should check to make sure that the path_length does not
    // exceed the bounds of the array.  We'll ignore that in this
    // example.

    // Simplify the learned path.
    simplify_path();

    // Display the path on the LCD.
    // display_path();
  }

  // Solved the maze!

  // Now enter an infinite loop - we can re-run the maze as many
  // times as we want to.

  while (1)
  {
    // Beep to show that we solved the maze.
    SetSpeeds(0, 0);
  //  OrangutanBuzzer::play(">>a32");
    Serial.println("End !!!");
    // Wait for the user to press a button, while displaying
    // the solution.
  //  while (!OrangutanPushbuttons::isPressed(BUTTON_B))
  //  {
  //    if (millis() % 2000 < 1000)
  //    {
  //      OrangutanLCD::clear();
  //      OrangutanLCD::print("Solved!");
  //      OrangutanLCD::gotoXY(0, 1);
  //      OrangutanLCD::print("Press B");
  //    }
  //    else
  //      display_path();
  //    delay(30);
  //  }
  //  while (OrangutanPushbuttons::isPressed(BUTTON_B));
  //  display.clearDisplay();
  //  display.setTextSize(2);
  //  display.setTextColor(WHITE);
  //  display.setCursor(20,0);
  //  display.println("AlhpaBot");
  //  display.setTextSize(3);
  //  display.setCursor(40,30);
  //  display.println("Go!");
  //  display.display();
    delay(500);

    value = 0;
    while(value != 0xEF)  //wait button pressed
    {
      PCF8574Write(0x1F | PCF8574Read());
      value = PCF8574Read() | 0xE0;
    }
    delay(1000);

    // Re-run the maze.  It's not necessary to identify the
    // intersections, so this loop is really simple.
    int i;
    for (i = 0; i < path_length; i++)
    {
      follow_segment();

      // Drive straight while slowing down, as before.
      // SetSpeeds(0, 0);
      // delay(100);
      SetSpeeds(30, 30);
      delay(100);

      // Make a turn according to the instruction stored in
      // path[i].
      turn(path[i]);
    }

    // Follow the last segment up to the finish.
    follow_segment();

    // Now we should be at the finish!  Restart the loop.
  }
}

void SetSpeeds(int Aspeed,int Bspeed)
{
  if(Aspeed < 0)
  {
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
    analogWrite(PWMA,-Aspeed);      
  }
  else
  {
    digitalWrite(AIN1,LOW); 
    digitalWrite(AIN2,HIGH);
    analogWrite(PWMA,Aspeed);  
  }
  
  if(Bspeed < 0)
  {
    digitalWrite(BIN1,HIGH);
    digitalWrite(BIN2,LOW);
    analogWrite(PWMB,-Bspeed);      
  }
  else
  {
    digitalWrite(BIN1,LOW); 
    digitalWrite(BIN2,HIGH);
    analogWrite(PWMB,Bspeed);  
  }
}

void PCF8574Write(byte data)
{
  Wire.beginTransmission(Addr);
  Wire.write(data);
  Wire.endTransmission(); 
}

byte PCF8574Read()
{
  int data = -1;
  Wire.requestFrom(Addr, 1);
  if(Wire.available()) {
    data = Wire.read();
  }
  return data;
}

// Place this code somewhere in your program for calibration purposes
// You can call this from setup() or use a button press to trigger it
void calibrateTurns() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Calibration Mode");
  display.setCursor(0,10);
  display.println("Press button for each test");
  display.display();
  
  // Wait for button press
  value = 0;
  while(value != 0xEF) {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
  }
  
  // Test 90-degree left turn
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Testing Left Turn...");
  display.display();
  delay(1000);
  SetSpeeds(-LRSpeeds0, LRSpeeds0);
  delay(LRDelay0);
  SetSpeeds(0, 0);
  
  // Wait for next test
  value = 0;
  while(value != 0xEF) {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
  }
  
  // Test 90-degree right turn
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Testing Right Turn...");
  display.display();
  delay(1000);
  SetSpeeds(LRSpeeds0, -LRSpeeds0);
  delay(LRDelay0);
  SetSpeeds(0, 0);
  
  // Wait for next test
  value = 0;
  while(value != 0xEF) {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
  }
  
  // Test U-turn
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Testing U-Turn...");
  display.display();
  delay(1000);
  SetSpeeds(BSpeeds0, -BSpeeds0);
  delay(BDelay0);
  SetSpeeds(0, 0);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Calibration Complete");
  display.display();
  delay(2000);
}
// This function is used to generate a color wheel effect