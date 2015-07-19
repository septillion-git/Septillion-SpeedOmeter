/*************************************************** 
  SpeedOmeter v1.1 - Speedometer for modelrailroad
  Created by Timo Engelgeer (Septillion), May 30, 2015
  Based on Speedometer bij Max Roodveldt
  
  Calculates the (scale) speed of a (model) train based on two 
  detections on the layout. This can be any kind you like.
  
****************************************************/
byte me;
#define SPEEDOMETER_VERSION "v1.1"
/***** Configuration ******************************* 
  The Speedometer is made very versatile. So in order to make it work with 
  your setup you have to configure it. The next few blocks will guide you to 
  the configuration.
  
  -- Display --
  Because you can use different displays with Speedometer you have to select
  the correct one.
  1 SSD1306 128x64 OLED display (I2C, default address 0x3C)
  2 Adafruit 7-segment display (I2C, default address is 0x70)

****************************************************/
#define LCD_TYPE 2

/**-- Sensor --
  It doesn't matter what kind of sensor you use. This can be an active low
  (signal goes low when train passes) or active high (signal goes high when
  train passes).
  
  Comment the line below for active low.
  #define ACTIVE_LOW    for active low  (uncommented)
  //#define ACTIVE_LOW  for active high (commented => not defined)
  
  Analoog komt nog ;)
**/
#define ACTIVE_LOW

/**-- Pins --
  To which pins are the sensors connected? Left and Right respectively
**/
const byte InputL = 9;
const byte InputR = 10;

/**-- Settings --
  Distance =>     Distance between start of the first detector and 
                  the start of the second (in mm)
  Scale =>        Scale of the layout
  TimeShowSpeed =>Time to show the speed after finishing the measurement (in ms)
  TimeOut =>      Time to ignore re-triggering the detector after 
                  finishing measurement (in ms) 
**/
unsigned int distance = 5000;
unsigned int scale = 87;

const unsigned int TimeShowSpeed = 5000;
const unsigned int TimeOut = 3000;

/**-- Done! --
  You are done. Upload the sketch!
**/

//SSD1306 based OLED display
#if (LCD_TYPE == 1)
  #include <SPI.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>

  Adafruit_SSD1306 display(255);
  const byte I2CAdress = 0x3C;
  
  #define INFO_EXTEND

//Adafruit 7-segment I2C
#else if (LCD_TYPE == 2)
  #include <Wire.h>
  #include "Adafruit_LEDBackpack.h"
  #include "Adafruit_GFX.h"

  Adafruit_7segment display = Adafruit_7segment();
  const byte I2CAdress = 0x70;
  const byte DisplayBrightness = 7; // 0..15
#endif

/***** Global vars *********************************/
bool timerRuns = false; //Does the timer run?
bool direction = false; //In which direction? (false = to left, true = to right
bool ready = true;      //ready to start the clock?
byte displayScreen = 0; //Which screen is displayed last?

unsigned long startTime;      //Saved start time
unsigned long endTime;        //Saved end time
unsigned long showTimer =  -1;//Time when display started showing something

//Speed stats
unsigned int speed, maxSpeed, minSpeed, avgSpeed = 0;

/***** Default functions ***************************/

void setup() {
  //Initialize the display
  displayInit();
  
  //Set the input acording to the setting
  #if defined(ACTIVE_LOW)
    pinMode(InputL, INPUT_PULLUP);
    pinMode(InputR, INPUT_PULLUP);
  #else
    pinMode(InputL, INPUT);
    pinMode(InputR, INPUT);
  #endif
}

void loop(){
  //Check for a finished measurement
  if(checkInput()){
    //If so, calculate the speed and display it
    speed = calcSpeed(endTime - startTime);
    displaySpeed(speed);
    
    //Set timer to display it
    showTimer = millis();
    
    //Calc and store stats, skip for small displays
    #ifdef INFO_EXTEND
      if(speed > maxSpeed){
        maxSpeed = speed;
      }
      if(speed < minSpeed || minSpeed == 0){
        minSpeed = speed;
      }
      if(avgSpeed == 0){
        avgSpeed = speed;
      }
      else{
        avgSpeed += speed + 1;
        avgSpeed /= 2;
      }
    #endif 
  }
  
  //As long as display timer runs we don't update the screen
  if(millis() - showTimer > TimeShowSpeed){
    //If timer runs, busy display
    if(timerRuns){
      displayBusy();
    }
    //or the idle display
    else{
      displayIdle();
    }
  }
}

/***** SpeedOmeter functions ***********************/

//Calculates the speed as a fixed point int (1 decimal), with correct rounding
unsigned int calcSpeed(unsigned long time){
  return (unsigned int)((36UL * distance * scale + (time / 2)) / time );
}

//Checks the inputs, returns true if a measurement is done, false otherwise
bool checkInput(){
  //state of the input, HIGH for detect (no matter the setting)
  bool lVal, rVal;
  
  //Set depending the setting
  #if defined(ACTIVE_LOW)
    lVal = !digitalRead(InputL);
    rVal = !digitalRead(InputR);
  #else
    lVal = digitalRead(InputL);
    rVal = digitalRead(InputR);
  #endif
  
  //Busy with timing
  if(timerRuns){
    //End when the other detector is reached
    //Save endTime, remove flag, return true because of end
    if(direction && lVal || !direction && rVal){
      endTime = millis();
      timerRuns = false;
      return true;
    }
  }
  //Not busy
  else{
    //We are ready and there is an input
    if((lVal || rVal) && ready){
      startTime = millis();
      timerRuns = true;
      ready = false;
      direction = rVal;
    }
    
    //Probably not ready, 
    //make ready if no input at least TimeOut after ending timer
    else if(!lVal && !rVal && (millis() - endTime > TimeOut)){
      ready = true;
    }
  }
  
  //As long as it's not the end of the timer
  return false;
}

/***** Display functions ***************************/

//Set up the display
void displayInit(){
  //SSD1306 based OLED display
  #if (LCD_TYPE == 1)
    //Setup, clear and welkom
    display.begin(SSD1306_SWITCHCAPVCC, I2CAdress);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("SpeedOmeter ");
    display.println(SPEEDOMETER_VERSION);
    display.println("Model railroad speed");
    display.println("By Timo Engelgeer");
    display.println("Septillion");
    display.print("Scale: ");
    display.println(scale);
    display.print("Dist: ");
    display.print(distance);
    display.display();
    delay(1000);

  //Adafruit 7-segment I2C
  #else if (LCD_TYPE == 2)
    byte icons[] = {0x5C, 0x63};
  
    // set display
    display.begin(I2CAdress);
    display.setBrightness((DisplayBrightness > 15 ? 15 : DisplayBrightness)); // 0..15
    
    //Start up animation
    for(byte i = 0; i < 4; i++){
      for(byte y = 0; y < 4; y++){
        display.writeDigitRaw(y + (y >= 2 ? 1 : 0), icons[(y + i) % 2] | (y == i ? 0x80 : 0x00));
      }
      display.writeDisplay();
      delay(500);
    }
  #endif
}

//Displays the speed
void displaySpeed(unsigned int speed){
  //SSD1306 based OLED display
  #if (LCD_TYPE == 1)
    byte position = 2; //to allign the speed right
  
    displayScreen = 2; //so we do this once
    
    //Clear etc
    display.clearDisplay();
    display.stopscroll();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
    
    //Calculate the position so it's aligned to the right
    for(unsigned int test = (speed / 100); test; test /= 10){
      position--;
    }
    
    //Print speed
    display.print("Speed:");
    display.setTextSize(4);
    display.setCursor(position * 24,16);
    display.print(speed/10);
    display.print(",");
    display.print(speed % 10);
    display.setCursor(80, 48);
    display.setTextSize(2);
    display.print("km/h");
    
    display.display();
    
  //Adafruit 7-segment I2C
  #else if (LCD_TYPE == 2)
    byte displayPos = 4; //Position on the screen
    
    displayScreen = 2;//so we do this once
    
    //Disable blink
    display.blinkRate(0);
    
    //But it speed is out of reach...
    if(speed > 9999){
      //Make speed 9999 and blink
      speed = 9999;
      display.blinkRate(1);
    }
    
    //Print speed
    if(speed){  //if speed is not 0
      for(byte i = 0; speed && i <= 4; ++i) {
        display.writeDigitNum(displayPos--, speed % 10, i == 1);
        if(displayPos == 2) display.writeDigitRaw(displayPos--, 0x00);
        speed /= 10;
      }
    }
    else {
      display.writeDigitNum(displayPos--, 0, false);
    }
    
    //Fill rest with 0
    while(displayPos < 100) display.writeDigitRaw(displayPos--, 0x00);
    display.writeDisplay();
  #endif
}

//Display when the timer is started
void displayBusy(){
  //SSD1306 based OLED display
  #if (LCD_TYPE == 1)
    if(displayScreen != 3){
      displayScreen = 3; //so we do this once
      
      //Clear etc
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      
      //If from right to left, scroll <--
      if(direction){
        for(byte i = 0; i < 2; i++){
          display.print("<--- ");
        }
        display.startscrollleft(0x00, 0x01);
      }
      //If from left to right, scroll -->
      else{
        for(byte i = 0; i < 2; i++){
          display.print("---> ");
        }
        display.startscrollright(0x00, 0x01);
      }
      
      //Display the info below
      displayInfo();
      display.display();
    }
    
  //Adafruit 7-segment I2C
  #else if (LCD_TYPE == 2)
    if(displayScreen != 3){
      displayScreen = 3; //so we do this once
      
      // |---- for from left, ----| for from right
      for(byte i = 0; i < 5; i++){
        if(i == 2) display.writeDigitRaw(i++, 0x00);
        if(i == 0 && !direction){
          display.writeDigitRaw(i, 0x70);
        }
        else if(i == 4 && direction){
          display.writeDigitRaw(i, 0x46);
        }
        else{
          display.writeDigitRaw(i, 0x40);
        }
      }
      //and blink 1Hz
      display.blinkRate(2);
      display.writeDisplay();
    }
  #endif
}

//Screen to display when waiting for input
void displayIdle(){
  //SSD1306 based OLED display
  #if (LCD_TYPE == 1)
    if(displayScreen != 4){
      displayScreen = 4; //so we do this once
      
      //Clear, don't scroll, set size, set cursor, write
      display.clearDisplay();
      display.stopscroll();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Wachten...");
      
      //And add info
      displayInfo();
      
      display.display();
    }
  
  
  //Adafruit 7-segment I2C
  #else if (LCD_TYPE == 2)
    if(displayScreen != 4){
      displayScreen = 4; //so we do this once
      display.blinkRate(0); //Don't blink
      
      // Write "____"
      for(byte i = 0; i < 5; i++){
        if(i == 2) display.writeDigitRaw(i++, 0x00);
        display.writeDigitRaw(i, 0x08);
      }
      display.writeDisplay();
    }
  #endif
}

void displayError(){
  
  displayScreen = 5;
  
}

//Display's the stats (min, max, avg, last)
void displayInfo(){
  #if (LCD_TYPE == 1)
    //Print last speed, max speed, min speed and average
    display.setTextSize(1);
    display.print("Lst: ");
    display.print(speed / 10);
    display.print(",");
    display.print(speed % 10);
    display.println("km/h");
    display.print("Max: ");
    display.print(maxSpeed / 10);
    display.print(",");
    display.print(maxSpeed % 10);
    display.println("km/h");
    display.print("Min: ");
    display.print(minSpeed / 10);
    display.print(",");
    display.print(minSpeed % 10);
    display.println("km/h");
    display.print("Avg: ");
    display.print(avgSpeed / 10);
    display.print(",");
    display.print(avgSpeed % 10);
    display.println("km/h");
  #endif
}