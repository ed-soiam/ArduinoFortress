#include <stdlib.h>
#include "ArduinoFortress.h"
ArduinoFortress * af;

void setup() 
{
  Serial.begin(115200);
  delay(10000);
  pinMode(13, OUTPUT);
  af = new ArduinoFortress();
}


void loop() 
{ 
  af -> proc();
}


ArduinoFortress::ArduinoFortress():
  gsm(Serial1),
  sh(Serial3),
  vs(0,450,true),//11.2v low on 5v power supply
  sent(false)
{
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
  //msg = NetroMessage::createStd(0x1220,0,0,0); 
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


void ArduinoFortress::proc()
{
  //vs.proc();
  gsm.proc(); 
  if (gsm.currentTask().isCompleted())
  {
    parseGSMTaskResults(gsm.currentTask());     
    gsm.clearCurrentTask();
  }
  sh.proc();
  /*
  if (sh.isFree() && next_cmd_time < millis())
  {
    sh.sendCommand(*msg);
    next_cmd_time = millis()  + 5000;
  }*/
}


void ArduinoFortress::parseGSMTaskResults(const GSMTask & task)
{
  if (task.isError())
    return;
  switch (task.task())
  {
  case GSMTask::GSM_TASK_READ_SMS:
    parseSMS(task.resultPhone(),task.resultText());
    break;
  default:
    break;
  }
}


void ArduinoFortress::parseSMS(const String & phone, const String & sms)
{
  GSMTask::GSM_SEND_SMS_T param;
  String sms_text = sms;
  param.phone = phone;
  sms_text.toLowerCase();
  //removing end symbols
  while (sms_text.charAt(sms_text.length() - 1) == '\n' || sms_text.charAt(sms_text.length() - 1) == '\r')
    sms_text.remove(sms_text.length() - 1);
  
  String sms_part[8];//cmd,subcmd and 6 parameters
  int start_substring = 0;
  bool any_symbols = false;
  int part_number = 0;
  for (int j = 0; j < sms.length(); j++)
    if (sms_text.charAt(j) == ' ' || j == sms.length() - 1)
    {
      if (any_symbols)
      {
        sms_part[part_number] = sms_text.substring(start_substring,j);
        part_number++;
        if (part_number >= 8)
          break;//too many words
      }
      start_substring = j + 1;
      any_symbols = false;  
    }
    else
    {
      any_symbols = true;
    }
    
  //command parser
  if (sms_part[0] == "sensor")
  {
    if (sms_part[1] == "listen")
    {
      param.text = "listen ok";   
    }
    else
      param.text = "listen subcommand error";
    gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param));
    return;
  }

  if (sms_part[0] == "phone")
  {

    return;
  }

  if (sms_part[0] == "memory")
  {

    return;
  }

  if (sms_part[0] == "report")
  {

    return;
  }
  if (sms_part[0] == "hello")
  {
    param.text = "Hello my dear friend!!!";
    gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param));
  }
  Serial.println(sms_part[0].c_str());
}


int ArduinoFortress::freeRam() 
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


void ArduinoFortress::saveTestPhone()
{
  EEPROMManager::PHONE_ELEMENT_T phone;
  memset(&phone,0,sizeof(phone));
  const char ph[] = "+375290000000";
  memcpy(phone.number,ph,sizeof(ph));
  EEPROMManager::save(EEPROMManager::EEPROM_PHONE_PART,0,(unsigned char *)&phone);
}
