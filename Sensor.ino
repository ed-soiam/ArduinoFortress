#include "Sensor.h"

Sensor::Sensor(SENSOR_T type):
_type(type),
_alarm(false),
_enabled(true)
{

}


void Sensor::setName(const char * pName)
{
  strncpy (_name, pName, sizeof(_name));
}

