#ifndef _SENSOR_H_
#define _SENSOR_H_
#include "defs.h"
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
  void setName(const char * pName);
  const char * getName() const {return _name;}
  SENSOR_T sensorType() const{return _type;}
  
  void setEnabled(bool value) {this -> _enabled = value ? 1 : 0;}
  virtual bool isEnabled() const {return _enabled;}
  
  //return true if alarm
  virtual bool proc() = 0;
  //setup sensor from raw data
  virtual bool fromEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data) = 0;  
  virtual bool toEEPROMData(EEPROMManager::SENSOR_ELEMENT_T * data) = 0;
  
  virtual bool isAlarm() const {return (_enabled && _alarm);}
  virtual void setAlarm(bool value) {this -> _alarm = value ? 1 : 0;}
  virtual String alarmMessage() = 0;
  //report of current Sensor state(with name)
  virtual String report() const = 0;
  //get id of sensor
  virtual unsigned long id() const = 0;

protected:  
  char _name[SENSOR_NAME_LENGTH];
  SENSOR_T _type;
  unsigned char _alarm;//activation of alarm
  unsigned char _enabled;//is sensor enabled?
};
#endif
