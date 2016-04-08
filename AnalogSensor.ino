#include "AnalogSensor.h"
AnalogSensor::AnalogSensor(const char * name, unsigned char pin, unsigned short compareValue, bool lowOn) :
Sensor(ANALOG_SENSOR)
{
  Serial.println("Analog sensor object was created");
  _pin = pin;
  setName(name);
  _compare = compareValue;
  _lowOn = lowOn;
  for (int i = 0; i < 10; i++)
    if (_lowOn)
      value[i] = _compare + 100;
    else
      value[i] = _compare - 100;
}

AnalogSensor::AnalogSensor(EEPROMManager::SENSOR_ELEMENT_T * data) : 
Sensor(ANALOG_SENSOR)
{
  fromEEPROMData(data);
  Serial.print("Analog sensor object was created: compare ");
  Serial.println(_compare);
}

bool AnalogSensor::proc()
{
  memmove(value,&value[1], sizeof(value) - sizeof(value[0]));
  value[9] = analogRead (_pin);
  unsigned long average = 0;
  for (int i = 0; i < 10; i++)
     average += value[i];
  average /= 10;
  if ((average < _compare && _lowOn) || (average > _compare && !_lowOn))
    setAlarm(true);
  return isAlarm();  
}


String AnalogSensor::alarmMessage()
{
  if (!isAlarm())
    return String();
  setAlarm(false);
  return report();
}


String AnalogSensor::report() const
{
  unsigned long average = 0;
  for (int i = 0; i < 10; i++)
     average += value[i];
  average /= 10;
  String s;
  s.reserve(32);
  s = getName();
  s.concat(" ");
  s.concat(String(average * 5 * RESIST_DIVIDER / 1024.0 , 2));
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

