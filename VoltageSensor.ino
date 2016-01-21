#include "VoltageSensor.h"
VoltageSensor::VoltageSensor(unsigned char pin)
{
  Serial.print("Voltage sensor object was created\n\r");
  this -> pin = pin;
}

void VoltageSensor::proc()
{
  unsigned short vsensor = analogRead (0); 
  Serial.print("Object voltage sensor ") ;
  Serial.print(vsensor * 5 / 1024.0);
  Serial.print(" volts\n\r");
}
