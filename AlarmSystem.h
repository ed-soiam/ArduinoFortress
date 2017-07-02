#ifndef _ALARM_SYSTEM_H_
#define _ALARM_SYSTEM_H_
#include "Sensor.h"
class AlarmSystem {
public:
  AlarmSystem(Sensor * sensors[], int count);
  String alarmSMS();
private:
  Sensor ** _sensors;
  int _count;
  unsigned long _last_alarm_sms_time;
  unsigned char _alarm_sms_frequency;   
};
#endif
