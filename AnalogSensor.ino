#include "AnalogSensor.h"
AnalogSensor::AnalogSensor(const char * name, unsigned char pin, unsigned short compareValue, bool lowOn) :
Sensor(ANALOG_SENSOR)
{
  Serial.println("Analog sensor object was created");
  _pin = pin;
  setName(name);
  _compare = compareValue;
  _lowOn = lowOn;
}

AnalogSensor::AnalogSensor(EEPROMManager::SENSOR_ELEMENT_T * data) : 
Sensor(ANALOG_SENSOR)
{
  fromEEPROMData(data);
}

bool AnalogSensor::proc()
{
  value = analogRead (_pin); 
  //Serial.print("Voltage sensor ") ;
  //Serial.print(vsensor * 5 / 1024.0);
  //Serial.print(" volts\n\r");
  return false;  
}


String AnalogSensor::alarmMessage() const
{
  return String();
}


String AnalogSensor::report() const
{
  String s;
  s.reserve(32);
  s = getName();
  s.concat(" ");
  s.concat(String(value * 5 / 1024.0 , 2));
  return s;
}


bool AnalogSensor::fromEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data)
{
  _pin = data -> analog_sensor.pin;
  _compare = data -> analog_sensor.compare;
  _lowOn = data -> analog_sensor.lowOn;
  setName((const char *)data -> name);
}


bool AnalogSensor::toEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data)
{
  memset(data,0,sizeof(*data));
  strncpy(data -> name, getName(),sizeof(data -> name));
  data -> type = _type;
  data -> analog_sensor.pin = _pin;
  data -> analog_sensor.compare = _compare;
  data -> analog_sensor.lowOn = _lowOn ? 1 : 0;
  return true;
}

