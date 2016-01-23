#ifndef _SENSOR_H_
#define _SENSOR_H_
class Sensor {
public: 
  typedef enum {
    UNKNOWN_SENSOR = 0,
    ANALOG_SENSOR,
    DIGITAL_SENSOR,
    I2C_SENSOR,
    REMOTE_SENSOR
  } SENSOR_T;
  Sensor();
  void setName(const String & s){_name = s;}
  String name() const {return _name;}
  SENSOR_T sensorType() const{return _type;}
  virtual void proc() = 0;
protected:
  
  String _name;
  SENSOR_T _type;
};
#endif
