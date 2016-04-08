#ifndef _ANALOG_SENSOR_H_
#define _ANALOG_SENSOR_H_
#include "Sensor.h"
class AnalogSensor : public Sensor
{
public:
  AnalogSensor(const char * name, unsigned char pin, unsigned short compareValue, bool lowOn);
  AnalogSensor(EEPROMManager::SENSOR_ELEMENT_T * data);
  bool proc();
  //setup sensor from raw data
  virtual bool fromEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data);
  virtual bool toEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data);
  virtual String alarmMessage();
  virtual String report() const;
  virtual unsigned long id() const {return _pin;}
private:
  unsigned char _pin; 
  unsigned short _compare;
  bool _lowOn;
  unsigned short value[10];
};
#endif
