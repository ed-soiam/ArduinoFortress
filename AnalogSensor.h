#ifndef _ANALOG_SENSOR_H_
#define _ANALOG_SENSOR_H_
#include "Sensor.h"
class AnalogSensor : public Sensor
{
public:
  AnalogSensor(unsigned char pin, unsigned short compareValue, bool lowOn);
  void proc();
private:
  unsigned char pin; 
};
#endif
