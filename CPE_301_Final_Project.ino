/*
Student 1: Kenneth Matthews
Student 2: Spencer Carter
Student 3: Khang Vo
*/

#include <LiquidCrystal.h>                                                             
#include "DHT.h"
#include "Stepper.h"
#include <Servo.h>
#define STEPS  32  
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>



volatile boolean TurnDetected;  
volatile boolean rotationdirection;  


const int PinCLK=2;   
const int PinDT=3;    
const int PinSW=4;    

int RotaryPosition=0;    

int PrevPosition;     
int StepsToTake;      

/ DHT-11
#define DHTPIN 6       
#define DHTTYPE DHT11   
// Setup DHT sensor 
DHT dht(DHTPIN, DHTTYPE);
// DHT variables
float hum;
float temp;


// Define Port D Register Pointers
volatile unsigned char* Port_D = (unsigned char*) 0x2B;
volatile unsigned char* ddr_D = (unsigned char*) 0x2A; 
volatile unsigned char* pin_D = (unsigned char*) 0x29;

// Define Port A Register Pointers
volatile unsigned char* Port_A = (unsigned char*) 0x22;
volatile unsigned char* ddr_A = (unsigned char*) 0x21; 
volatile unsigned char* pin_A = (unsigned char*) 0x20;

// Define Port C Register Pointers
volatile unsigned char* Port_C = (unsigned char*) 0x28;
volatile unsigned char* ddr_C = (unsigned char*) 0x27;
volatile unsigned char* pin_C = (unsigned char*) 0x26;

// Define Port H Register Pointers
volatile unsigned char* Port_H = (unsigned char*) 0x102;
volatile unsigned char* ddr_H = (unsigned char*) 0x101;
volatile unsigned char* pin_H = (unsigned char*) 0x100;

void Running_State();
void Idle_State();
void Error_State();
void Stepper_Motor();
void Disabled_State();
void RTC_TIMMER();
void digitalClockDisplay();
void printDigits(int digits);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 5, en = 19, d4 = 18, d5 = 17, d6 = 16, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int Start_Button;
int state;
const int disable = 0;
const int run = 1;
const int idle = 2;
const int error = 3;


float Temp_Thresh = 25;
float Humid_Thresh;
float Water_Thresh = 225;

Stepper small_stepper(STEPS, 8, 10, 9, 11 );

/////////ISR for Stepper Motor/////
void isr ()  {
  delay(4);  // delay for Debouncing
  if (digitalRead(PinCLK))
    rotationdirection= digitalRead(PinDT);
  else
  rotationdirection= !digitalRead(PinDT);
  TurnDetected = true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Void SetUp /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() 
{
  
    // Serial begin
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // initialize DHT
  dht.begin();


  // Port SETUP //
  // ***'1'= output '0'=input ***//

      // PH4 Setup. (output) For Fan
      *ddr_H |= 0b00010000;
      
      // PA0 // Blue Run LED
      *ddr_A |= 0b00000001;
      // PA2 // Green Idle  LED
      *ddr_A |= 0b00000100;
      // PA4 // Red Error LED
      *ddr_A |= 0b00010000;
      // PA6 // Yellow Disabled LED
      *ddr_A |= 0b01000000;

      // PC7 Setup. (input)
      *ddr_C &= 0b01111111;
  
  pinMode(PinCLK,INPUT);
  pinMode(PinDT,INPUT);  
  pinMode(PinSW,INPUT);
  digitalWrite(PinSW, HIGH); 
  attachInterrupt (0,isr,FALLING) ;
  myservo.attach(9);
  myservo.write(RotaryPosition);
  
  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Void Loop //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop(){
  
  switch (state) {
  case disable:
    Disabled_State();
    Stepper_Motor();
    break;

  case run:
    Running_State();
    Stepper_Motor();
    break;

  case idle:
    Idle_State();
    Stepper_Motor();
    break;

  case error:
    Error_State();
    Stepper_Motor();
    break;

  default:
    break; 
    }
  
 
  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Stepper_Motor()
{
  //Stepper_Motor_Loop
  small_stepper.setSpeed(100); 
  if (!(digitalRead(PinSW))) {   
    if (RotaryPosition == 0) {  
    } else {
        small_stepper.step(-(RotaryPosition*50));
        RotaryPosition=0; 
      }
    }

  // Runs if rotation was detected
  if (TurnDetected)  {
    PrevPosition = RotaryPosition; 
    if (rotationdirection) {
      RotaryPosition=RotaryPosition-1;} 
    else {
      RotaryPosition=RotaryPosition+1;} 

    TurnDetected = false;  

    // Motor Direction
    if ((PrevPosition + 1) == RotaryPosition) { 
      StepsToTake=50; 
      small_stepper.step(StepsToTake);
    }

    if ((RotaryPosition + 1) == PrevPosition) { 
      StepsToTake=-50;
      small_stepper.step(StepsToTake);
    }
    
    Serial.print("Position: ");
    Serial.println(RotaryPosition);
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}
void RTC_TIMMER()
{
  if (timeStatus() == timeSet) {
    digitalClockDisplay();
  } else {
    Serial.println("The time has not been set.  Please run the Time");
    Serial.println();
    delay(4000);
  }
  //delay(1000);
}

void digitalClockDisplay(){

  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void Running_State()
{
  Serial.println("Run State"); 
  // Stability Delay
   delay(2000); 

  // Get humidity and Temp from sensor
  hum = dht.readHumidity();  // Get Humidity value
  temp = dht.readTemperature();  // Get Temperature value
    
  // Clear the display
  lcd.clear();
    
  // Print temperature on top line
  lcd.setCursor(0,0);
  lcd.print("Temp:  ");
  lcd.print(temp);
  lcd.print(" C");
  
  // Print humidity on bottom line columbs / ¨rows
  lcd.setCursor(0,1);
  lcd.print("Humid: ");
  lcd.print(hum);
  lcd.print(" %");
  // Validate Run 
  if (temp >=  Temp_Thresh)
    {
    *Port_H |= 0b00010000; // Turns Fan On
    *Port_A &= 0b10000000; // Turns OF LED 
    *Port_A |= 0b00000001; // Turns Blue LED ON
    }
    else
    {
      state = idle;
    } 
    // Measures Water Level 
    if(analogRead(A0) <= Water_Thresh)
    {
    *Port_D &= 0b11111110; // stop fan for fan
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("Water LOW");
    Serial.print("Water level low!");
    Serial.print('\n');
    state = error;
    RTC_TIMMER();
    delay(500);
    }
     Start_Button = *pin_C & 0b01111111; 
    if(Start_Button)
    {
      RTC_TIMMER();
      state = disable;
    }  
  

}

void Idle_State()
{
  
  Serial.println("IDLE State");
// Stability Delay
   delay(2000); 

  // Get humidity and Temp from sensor
  hum = dht.readHumidity();  // Get Humidity value
  temp = dht.readTemperature();  // Get Temperature value
    
  // Clear the display
  lcd.clear();
    
  // Print temperature on top line
  lcd.setCursor(0,0);
  lcd.print("Temp:  ");
  lcd.print(temp);
  lcd.print(" C");
  
  // Print humidity on bottom line columbs / ¨rows
  lcd.setCursor(0,1);
  lcd.print("Humid: ");
  lcd.print(hum);
  lcd.print(" %");
  // Validate Run 
  if (temp >=  Temp_Thresh)
    {
    RTC_TIMMER();  
    state = run;
    lcd.display();
    
    }
    else
    {
      *Port_H &= 0b11101111; // Turns Fan Off
      *Port_A &= 0b10000000; // Turns OF ALL LEDs 
      *Port_A |= 0b00000100; // Turns Blue LED ON
    }
     
    // Measures Water Level 
    if(analogRead(A0) <= Water_Thresh)
    {
    *Port_H &= 0b11101111; // Turns Fan Off
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("Water LOW");

    RTC_TIMMER();
    state = error;
    delay(500);
    }
    Start_Button = 0;
   Start_Button = *pin_C & 0b01111111; 
    if(Start_Button)
    {
      RTC_TIMMER();
      state = disable;
    }  
    

}

void Error_State()
{
  Serial.println("ERROR State");
 // Measures Water Level 
    if(analogRead(A0) <= Water_Thresh)
    {
    *Port_A &= 0b10000000; // Turns OF ALL LEDs 
    *Port_A |= 0b00010000; // Turns Blue LED ON 
    *Port_H &= 0b11101111; // Turns Fan Off
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ERROR");
    lcd.setCursor(0,1);
    lcd.print("Water LOW");
    Serial.print('\n');
    //xstate = error;
    delay(500);
    }
    else 
    {
      state = disable;
      RTC_TIMMER();
    }

    Start_Button = *pin_C & 0b01111111; 
    if(Start_Button)
    {
      RTC_TIMMER();
      state = disable;
    }  
    
}

void Disabled_State()
{
  //delay(500);
  Serial.println("DISABLED State");
  // Clear screen and No Display
  lcd.clear();
  
  // Clear all LEDs 
  *Port_A &= 0b10000000; // Turns OF ALL LEDs 
  // Turn on Yellow LED
  *Port_A |= 0b01000000;
  // stop fan 
  *Port_H &= 0b11101111;  

  // Wait for On Button
  delay(500);
  Start_Button = *pin_C & 0b01111111; 
    if(Start_Button)
    {
      RTC_TIMMER();
      state = run;
    }  
  
}
