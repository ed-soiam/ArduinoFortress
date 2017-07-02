#ifndef _ARDUINOFORTRESS_H_
#define _ARDUINOFORTRESS_H_
#include "NetroMessage.h"
#include "Sensor.h"
#include "GSMModule.h"
#include "SHModem.h"
#include "EEPROMManager.h"
#include "AlarmSystem.h"
class ArduinoFortress
{
public:
  ArduinoFortress();
  //main periodical function
  void proc();
  //get total free ram
  int freeRam();
  //temp function to save phone number to EEPROM
  void saveTestPhone();
  void parseGSMTaskResults(const GSMTask & task); 
  void parseSMS(const char * phone, const String & sms);
  void printStackHeap();
  void alarm(unsigned long id);
private:
  SHModem sh;
  GSMModule gsm;
  Sensor * sensor[SENSOR_COUNT];
  AlarmSystem * _alarm_sys;
  bool _listen_mode;//listen to sensors mode;
  char _listen_phone[PHONE_NUMBER_LENGTH];//phone, which set listen to sensors mode
};
#endif

