#include "AnalogSensor.h"
AnalogSensor::AnalogSensor(unsigned char pin, unsigned short compareValue, bool lowOn)
{
  Serial.print("Analog sensor object was created\n\r");
  this -> pin = pin;
  _type = ANALOG_SENSOR;
}

void AnalogSensor::proc()
{
  unsigned short vsensor = analogRead (0); 
  Serial.print("Object voltage sensor ") ;
  Serial.print(vsensor * 5 / 1024.0);
  Serial.print(" volts\n\r");
}
