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

void setup() 
{
  

  
  pinMode(PinCLK,INPUT);
  pinMode(PinDT,INPUT);  
  pinMode(PinSW,INPUT);
  digitalWrite(PinSW, HIGH); // Pull-Up resistor for switch
  attachInterrupt (0,isr,FALLING) ;
  myservo.attach(9);
  myservo.write(RotaryPosition);
  
  
}

void loop(){
 
  
}


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
    if ((PrevPosition + 1) == RotaryPosition) { // Clockwise
      StepsToTake=50; 
      small_stepper.step(StepsToTake);
    }

    if ((RotaryPosition + 1) == PrevPosition) { // CounterClockwise
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
