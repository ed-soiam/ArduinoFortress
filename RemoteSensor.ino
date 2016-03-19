#include "RemoteSensor.h"

#include "AnalogSensor.h"
RemoteSensor::RemoteSensor(const char * name, unsigned long id) :
Sensor(REMOTE_SENSOR)
{
  Serial.println("Remote sensor object was created");
  _id = id;
  setName(name);
}

RemoteSensor::RemoteSensor(EEPROMManager::SENSOR_ELEMENT_T * data) : 
Sensor(REMOTE_SENSOR)
{
  fromEEPROMData(data);
}

bool RemoteSensor::proc()
{
  return false;
}


String RemoteSensor::alarmMessage() const
{
  return String();
}


String RemoteSensor::report() const
{
  return getName() + ' ' + String(value ? "on" : "off");
}


bool RemoteSensor::fromEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data)
{
  _id = data -> id;
  setName((const char *)data -> name);
}


bool RemoteSensor::toEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data)
{
  memset(data,0,sizeof(*data));
  strncpy(data -> name, getName(),sizeof(data -> name));
  data -> type = _type;
  data -> id = _id;
}

