
#include "Stepper.h"
#include <Servo.h>





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
