#ifndef _SENSOR_H_
#define _SENSOR_H_
#include "EEPROMManager.h"
class Sensor {
public: 
  typedef enum {
    UNKNOWN_SENSOR = 0,
    ANALOG_SENSOR,
    DIGITAL_SENSOR,
    I2C_SENSOR,
    REMOTE_SENSOR
  } SENSOR_T;
  Sensor(SENSOR_T type);
  void setName(const String & s){_name = s;}
  String getName() const {return _name;}
  SENSOR_T sensorType() const{return _type;}
  //return true if alarm
  virtual bool proc() = 0;
  //setup sensor from raw data
  virtual bool fromEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data) = 0;  
  virtual bool toEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data) = 0;
  
  virtual bool isAlarm() const {return alarm;};
  virtual void setAlarm(bool value) {this -> alarm = alarm;}
  virtual String alarmMessage() const = 0;
  //report of current Sensor state(with name)
  virtual String report() const = 0;
protected:  
  String _name;
  SENSOR_T _type;
  bool alarm;
};
#endif
