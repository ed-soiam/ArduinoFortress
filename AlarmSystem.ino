#include "AlarmSystem.h"
AlarmSystem::AlarmSystem(Sensor * sensors[], int count):
_sensors(sensors),
_count(count),
_last_alarm_sms_time(0),
_alarm_sms_frequency(0) 
{
  
}


String AlarmSystem::alarmSMS()
{
  String sms;
  unsigned long delta_time = 0;
  unsigned long current_time = millis();
  bool alarm = false;
  for (int i = 0; i < _count; i++)
    if (_sensors[i])
      alarm |= _sensors[i] -> proc();   

  switch(_alarm_sms_frequency)
  {
  case 0:case 1:case 2: break;
  case 3: delta_time = 30000;break;//30 sec
  case 4: delta_time = 60000;break;//1 min
  case 5: delta_time = 120000;break;//2 min
  case 6: delta_time = 600000;break;//10 min
  case 7: delta_time = 600000 * 6;break;//1 hour
  case 8: delta_time = 600000 * 6 * 6;break;//6 hour
  default: delta_time = 600000 * 6 * 24;break;//24 hour
  }
  
  if (!alarm)
  {
    if (!delta_time)
      delta_time = 30000;
    if (_last_alarm_sms_time + delta_time <=  current_time)
      _alarm_sms_frequency = 0;//frequency reset after long time of "no alarm"
    return sms;
  } 
  
  if (_last_alarm_sms_time + delta_time <=  current_time)
  {
    _last_alarm_sms_time = current_time;
    _alarm_sms_frequency += 1;//increase sms frequency variable  to increase delay of new sms sending process

    //send alarm sms 
    sms.reserve(140);
    for (int i = 0; i < SENSOR_COUNT; i++)
      if (_sensors[i] && _sensors[i] -> isAlarm())
      {
        if (sms.length())
          sms.concat(", ");            
        sms.concat(_sensors[i] -> alarmMessage());
      }
  }
  return sms;
}

