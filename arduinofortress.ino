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
int freeRam();

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
  GSMTask task;
  //prepare SIM module text mode
  task = GSMTask(GSMTask::GSM_TASK_SET_SMS_MODE,0);
  gsm.addTask(task);
  task = GSMTask(GSMTask::GSM_TASK_SET_GSM_ENCODING,0);
  gsm.addTask(task);
  //clearing SIM module memory from all sms
  task = GSMTask(GSMTask::GSM_TASK_DELETE_ALL_SMS,0);
  gsm.addTask(task);
}


unsigned long next_cmd_time = 0;
bool sent =false;
void loop() 
{ 
  //vs.proc();
  gsm.proc(); 
  if (!sent)
  {
    sent = true;
    GSMTask task;
    task = GSMTask(GSMTask::GSM_TASK_GET_REGISTERED,0);
    gsm.addTask(task);
    GSMTask::GSM_SEND_SMS_T param2;
    param2.text = "Hello";
    param2.phone = "+3750000000";
    task = GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param2);
    gsm.addTask(task);
  }
  if (gsm.currentTask().isCompleted())
  {
    if (gsm.currentTask().isError())
      Serial.println("GSM TASK error");
    else
    {
      Serial.println(freeRam());
      Serial.println("GSM TASK ok");
      Serial.println(gsm.currentTask().resultPhone().c_str());
      Serial.println(gsm.currentTask().resultText().c_str());      
    }
    gsm.clearCurrentTask();
  }
  /*sh.proc();
  if (sh.isFree() && next_cmd_time < millis())
  {
    sh.sendCommand(*msg);
    next_cmd_time = millis()  + 5000;
  }*/
}

int freeRam () 
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setupSensor()
{

}

