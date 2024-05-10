#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "TRSensors.h"
#include <Wire.h>
#include "EEPROM.h"

//Macros for motor speeds
#define LRSpeeds0 80
#define LRDelay0 180
#define BSpeeds0 100
#define BDelay0 280



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
      SetSpeeds(50,-50);
    }
    else
    {
     digitalWrite(AIN2,LOW);
     digitalWrite(AIN1,HIGH);
     digitalWrite(BIN1,HIGH); 
     digitalWrite(BIN2,LOW);  
        SetSpeeds(-50,50);
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

// This function, causes the 3pi to follow a segment of the maze until
// it detects an intersection, a dead end, or the finish.
void follow_segment()
{
  int last_proportional = 0;
  long integral=0;

  while(1)
  {
    // Normally, we will be following a line.  The code below is
    // similar to the 3pi-linefollower-pid example, but the maximum
    // speed is turned down to 60 for reliability.

    // Get the position of the line.
    unsigned int position = trs.readLine(sensorValues);

    // The "proportional" term should be 0 when we are on the line.
    int proportional = ((int)position) - 2000;

    // Compute the derivative (change) and integral (sum) of the
    // position.
    int derivative = proportional - last_proportional;
    integral += proportional;

    // Remember the last position.
    last_proportional = proportional;

    // Compute the difference between the two motor power settings,
    // m1 - m2.  If this is a positive number the robot will turn
    // to the left.  If it is a negative number, the robot will
    // turn to the right, and the magnitude of the number determines
    // the sharpness of the turn.
    int power_difference = proportional/20 + integral/10000 + derivative*10;

    // Compute the actual motor settings.  We never set either motor
    // to a negative value.
    int maximum;
    if (solved)
    {
      maximum = 100; // the maximum speed
    }else{
      maximum = 70; // learning speed
    }

    if (power_difference > maximum)
      power_difference = maximum;
    if (power_difference < -maximum)
      power_difference = - maximum;

    if (power_difference < 0)
    {
      analogWrite(PWMA,maximum + power_difference);
      analogWrite(PWMB,maximum);
    }
    else
    {
      analogWrite(PWMA,maximum);
      analogWrite(PWMB,maximum - power_difference);
    }

    // We use the inner three sensors (1, 2, and 3) for
    // determining whether there is a line straight ahead, and the
    // sensors 0 and 4 for detecting lines going to the left and
    // right.
   if(millis() - lasttime > 100)//100
   {
    if (sensorValues[1] < 150 && sensorValues[2] < 150 && sensorValues[3] < 150)
    {
      // There is no line visible ahead, and we didn't see any
      // intersection.  Must be a dead end.
      SetSpeeds(0,0);
      return;
    }
    else if (sensorValues[0] > 500 || sensorValues[4] > 500)
    {
      // Found an intersection.
      SetSpeeds(0, 0);
      return;
    }
   }
  }
}

// Code to perform various types of turns according to the parameter dir,
// which should be 'L' (left), 'R' (right), 'S' (straight), or 'B' (back).
// The delays here had to be calibrated for the 3pi's motors.
void turn(unsigned char dir)
{
  if(!solved)
  {
    switch(dir)
    {
    case 'L':
      // Turn left. 250-80- ,250 -120
      //              175-105, 250-120
      SetSpeeds(-LRSpeeds0, LRSpeeds0);
      delay(LRDelay0);
      // OrangutanBuzzer::play(">>a32");
      break;
    case 'R':
      // Turn right.
      SetSpeeds(LRSpeeds0, -LRSpeeds0);
      delay(LRDelay0);
      // OrangutanBuzzer::play(">>a32");
      break;
    case 'B':
      // Turn around.
      SetSpeeds(BSpeeds0, -BSpeeds0);
      delay(BDelay0);
      // OrangutanBuzzer::play(">>a32");
      break;
    case 'S':
      // Don't do anything!
      break;
    }
  }else
  {
    switch(dir)
    {
    case 'L':
      // Turn left. 250-80- ,250 -120
      //              175-105, 250-120
      SetSpeeds(-LRSpeeds0, LRSpeeds0);
      delay(LRDelay0);
      // OrangutanBuzzer::play(">>a32");
      break;
    case 'R':
      // Turn right.
      SetSpeeds(LRSpeeds0, -LRSpeeds0);
      delay(LRDelay0);
      // OrangutanBuzzer::play(">>a32");
      break;
    case 'B':
      // Turn around.
      SetSpeeds(BSpeeds0, -BSpeeds0);
      delay(BDelay0);
      // OrangutanBuzzer::play(">>a32");
      break;
    case 'S':
      // Don't do anything!
      break;
    }
  }
  SetSpeeds(0, 0);
  delay(50);
 // value = 0;
//  while(value != 0xEF)  //wait button pressed
//  {
//    PCF8574Write(0x1F | PCF8574Read());
//    value = PCF8574Read() | 0xE0;
//  }
//  Serial.write(dir);
//  Serial.println();

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(50,25);
  display.println((char)dir);
  display.display();

  lasttime = millis();   
}

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
  }else
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





unsigned char times_length = 0;


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
    SetSpeeds(30, 30);
    delay(40);

    // These variables record whether the robot has seen a line to the
    // left, straight ahead, and right, whil examining the current
    // intersection.
    unsigned char found_left = 0;
    unsigned char found_straight = 0;
    unsigned char found_right = 0;

    // Now read the sensors and check the intersection type.
   trs.readLine(sensorValues);

    // Check for left and right exits.
    if (sensorValues[0] > 500)
      found_left = 1;
    if (sensorValues[4] > 500)
      found_right = 1;

      // Drive straight a bit more - this is enough to line up our
      // wheels with the intersection.                                                                                                         // 40 -380
    
    SetSpeeds(20,20);
    delay(100);
    SetSpeeds(0, 0);
    delay(50);

    // Check for a straight exit.
    trs.readLine(sensorValues);
    if (sensorValues[1] > 500 || sensorValues[2] > 500 || sensorValues[3] > 500)
      found_straight = 1;

    // Check for the ending spot.
    // If all three middle sensors are on dark black, we have
    // solved the maze.
    if (sensorValues[0] > 500 && sensorValues[1] > 500 && sensorValues[2] > 500 && sensorValues[3] > 500 && sensorValues[4] > 500)
    {
      // SetSpeeds(-40, 40);
      // delay(500);
      // SetSpeeds(40, -40);
      // delay(1000);
    //   if (sensorValues[0] > 500 && sensorValues[1] > 500 && sensorValues[2] > 500 && sensorValues[3] > 500 && sensorValues[4] > 500)
    // {
    //   solved = 1;
    //   SetSpeeds(0, 0);
    //   break;
    // }

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
