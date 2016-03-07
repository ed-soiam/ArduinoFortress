#ifndef _ARDUINOFORTRESS_H_
#define _ARDUINOFORTRESS_H_
#include "NetroMessage.h"
#include "AnalogSensor.h"
#include "GSMModule.h"
#include "SHModem.h"
#include "EEPROMManager.h"

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
  void parseSMS(const String & phone, const String & sms);
private:
  SHModem sh;
  GSMModule gsm;
  AnalogSensor vs;
  NetroMessage * msg;
  bool sent;
};
#endif

