#include <stdlib.h>
#include "NetroMessage.h"
#include "AnalogSensor.h"
#include "GSMModule.h"
#include "SHModem.h"
#include "EEPROMManager.h"

NetroMessage * msg;
AnalogSensor vs(0,450,true);//11.2v low on 5v power supply
GSMModule gsm(Serial1);
SHModem sh(Serial3);

void saveTestPhone()
{
  EEPROMManager::PHONE_ELEMENT_T phone;
  memset(&phone,0,sizeof(phone));
  const char ph[] = "+375290000000";
  memcpy(phone.number,ph,sizeof(ph));
  EEPROMManager::save(EEPROMManager::EEPROM_PHONE_PART,0,(unsigned char *)&phone);
}

void setup() 
{
  Serial.begin(115200);
  delay(10000);
  pinMode(13, OUTPUT);
  //saveTestPhone();
  //load phone settings from eeprom
  EEPROMManager::PHONE_ELEMENT_T phone;
  for (unsigned char i = 0; i < PHONE_NUMBER_COUNT; i++)
  {
    EEPROMManager::load(EEPROMManager::EEPROM_PHONE_PART,i,(unsigned char *)&phone);
    if (phone.number[0] != 0xff && phone.number[0] != 0x00)
      gsm.setPhone(i,String((const char *)phone.number));
  } 
  
  gsm.begin(115200);
  sh.begin(115200);
  msg = NetroMessage::createStd(0x1220,0,0,0);
  //gsm.isAttached();
   
  //gsm.sendSMS("","hello");
}
unsigned long next_cmd_time = 0;
void loop() 
{ 
  //vs.proc();
  gsm.proc(); 
  /*sh.proc();
  if (sh.isFree() && next_cmd_time < millis())
  {
    sh.sendCommand(*msg);
    next_cmd_time = millis()  + 5000;
  }*/
}

void setupSensor()
{

}

