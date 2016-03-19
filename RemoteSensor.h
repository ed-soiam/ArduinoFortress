#ifndef _REMOTE_SENSOR_H_
#define _REMOTE_SENSOR_H_
#include "Sensor.h"
class RemoteSensor : public Sensor
{
public:
  RemoteSensor(const char * name, unsigned long id);
  RemoteSensor(EEPROMManager::SENSOR_ELEMENT_T * data);
  bool proc();
  //setup sensor from raw data
  virtual bool fromEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data);
  virtual bool toEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data);
  virtual String alarmMessage() const;
  virtual String report() const;
  virtual unsigned long id() const {return _id;}
private:
  unsigned long _id;
  bool value; 
};
#endif
