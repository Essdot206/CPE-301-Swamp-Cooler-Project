////////////////////////////////////// Final Project //////////////////////////////////////|
#include <LiquidCrystal.h>                                                             
#include "DHT.h"
#include "Stepper.h"
#include <Servo.h>
#define STEPS  32  
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 5, en = 19, d4 = 18, d5 = 17, d6 = 16, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// DHT-11
#define DHTPIN 6       // DHT-11 Output Pin connection
#define DHTTYPE DHT11   // DHT Type is DHT 11 (AM2302)
// Setup DHT sensor for normal 16mhz Arduino
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

// Water Sensor Value
int sensorValue = 0;

// Threasholds
float Temp_Thresh = 25;
float Humid_Thresh;
float Water_Thresh = 225;

// Funstions
void Running_State();
void Idle_State();
void Error_State();
void Stepper_Motor();
void Disabled_State();
void RTC_TIMMER();
void digitalClockDisplay();
void printDigits(int digits);


int Start_Button;
int state;
const int disable = 0;
const int run = 1;
const int idle = 2;
const int error = 3;


volatile boolean TurnDetected;  // Specific For interupts
volatile boolean rotationdirection;  // Clockwise or Counter Clock wise

const int PinCLK=2;   // Generating interrupts using CLK signal
const int PinDT=3;    // Reading DT signal
const int PinSW=4;    // Reading Push Button switch

int RotaryPosition=0;    // To store Stepper Motor Position

int PrevPosition;     // Previous Rotary position Value to check accuracy
int StepsToTake;      // How much to move Stepper

// Setup of proper sequencing for Motor Driver Pins
// In1, In2, In3, In4 in the sequence 1-3-2-4
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

const int BLUE = 22;
 int True = 0;
  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Void SetUp /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() 
{

      
  
  while (!Serial) ; // wait until Arduino Serial Monitor opens
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");

  //Stepper Motor pin modes
  pinMode(PinCLK,INPUT);
  pinMode(PinDT,INPUT);  
  pinMode(PinSW,INPUT);
  digitalWrite(PinSW, HIGH); // Pull-Up resistor for switch
  attachInterrupt (0,isr,FALLING) ;
  myservo.attach(9);
  myservo.write(RotaryPosition);

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
      


     

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Void Loop //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
//Wait for button press to execute commands
  
  
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
    // if nothing else matches, do the default
    // default is optional
    break;
}

  

}


  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void Stepper_Motor()
{
  //Stepper_Motor_Loop
  small_stepper.setSpeed(100); //Max seems to be 700
  if (!(digitalRead(PinSW))) {   // check if button is pressed
    if (RotaryPosition == 0) {  // check if button was already pressed
    } else {
        small_stepper.step(-(RotaryPosition*50));
        RotaryPosition=0; // Reset position to ZERO
      }
    }

  // Runs if rotation was detected
  if (TurnDetected)  {
    PrevPosition = RotaryPosition; // Save previous position in variable
    if (rotationdirection) {
      RotaryPosition=RotaryPosition-1;} // decrase Position by 1
    else {
      RotaryPosition=RotaryPosition+1;} // increase Position by 1

    TurnDetected = false;  // do NOT repeat IF loop until new rotation detected

    // Which direction to move Stepper motor
    if ((PrevPosition + 1) == RotaryPosition) { // Move motor CW
      StepsToTake=50; 
      small_stepper.step(StepsToTake);
    }

    if ((RotaryPosition + 1) == PrevPosition) { // Move motor CCW
      StepsToTake=-50;
      small_stepper.step(StepsToTake);
    }
    //myservo.write(RotaryPosition);
    Serial.print("Position: ");
    Serial.println(RotaryPosition);
  }
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

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
