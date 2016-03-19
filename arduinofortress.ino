#include <stdlib.h>
#include "ArduinoFortress.h"
#include "AnalogSensor.h"
#include "RemoteSensor.h"
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
  _listen_mode(false)
{
  memset(_listen_phone,0,sizeof(_listen_phone));
  //saveTestPhone();
  //load phone settings from eeprom  
  for (unsigned char i = 0; i < PHONE_NUMBER_COUNT; i++)
  {
    EEPROMManager::PHONE_ELEMENT_T phone;
    EEPROMManager::load(EEPROMManager::EEPROM_PHONE_PART,i,(unsigned char *)&phone);
    if (phone.number[0] != 0xff && phone.number[0] != 0x00)
      gsm.setPhone(i,phone.number);
  } 
  //load sensors info from eeprom
  for (unsigned char i = 0; i < SENSOR_COUNT; i++)
  {
    EEPROMManager::SENSOR_ELEMENT_T raw_sensor;
    EEPROMManager::load(EEPROMManager::EEPROM_SENSOR_PART,i,(unsigned char *)&raw_sensor);
    //factory needed :)
    switch (raw_sensor.type)
    {
    case Sensor::ANALOG_SENSOR:
      Serial.println("AnalogSensor in cell ");
      Serial.println(i);
      Serial.flush();     
      sensor[i] = new AnalogSensor(&raw_sensor);
      break;
    case Sensor::DIGITAL_SENSOR:
    case Sensor::I2C_SENSOR:
    case Sensor::REMOTE_SENSOR:
      sensor[i] = NULL;
      break;
    default:
      if (i == 0)//create analog voltage sensor if no sensor is presented in cell 0
      {
        Serial.println("Creating new Power voltage sensor");
        sensor[i] = new AnalogSensor("Power voltage",0,450,true);//11.2v low on 5v power supply
        sensor[i] -> toEEPROMData(&raw_sensor);
        EEPROMManager::save(EEPROMManager::EEPROM_SENSOR_PART,i,(unsigned char *)&raw_sensor);
      }
      else
      {
        Serial.print("No sensor in cell ");
        Serial.println(i);
        Serial.flush();
        sensor[i] = NULL;//no sensor in eeprom cell
      }
      break;  
    }
  }
   
  gsm.begin(115200);
  sh.begin(115200);
  GSMTask task;
  //clearing SIM module memory from all sms
  task = GSMTask(GSMTask::GSM_TASK_DELETE_ALL_SMS,0);
  gsm.addTask(task);
  //prepare SIM module text mode
  task = GSMTask(GSMTask::GSM_TASK_SET_SMS_MODE,0);
  gsm.addTask(task);
  task = GSMTask(GSMTask::GSM_TASK_SET_GSM_ENCODING,0);
  gsm.addTask(task);
  
}


void ArduinoFortress::proc()
{
  //sensors
  for (int i = 0; i < SENSOR_COUNT; i++)
    if (sensor[i])
      sensor[i] -> proc();
      
  //gsm    
  gsm.proc();
  GSMTask task = gsm.currentTask(); 
  if (task.isCompleted())
  {
    parseGSMTaskResults(gsm.currentTask());     
    gsm.clearCurrentTask();
  }
  
  //smarthome modem
  sh.proc();
  if (_listen_mode && !sh.isListenMode())
  {
    GSMTask::GSM_SEND_SMS_T param;
    param.text.reserve(64);
    memcpy(param.phone,_listen_phone,sizeof(param.phone));
    Serial.println(_listen_phone);
    Serial.flush();
    _listen_mode = false;
    if (sh.lastSensorId() == (unsigned long)(-1))
       param.text = "listen timeout";
    else
    {
      //searching for existing sensors
      for (int i = 0; i < SENSOR_COUNT; i++)
        if (sensor[i] && sensor[i] -> sensorType() == Sensor::REMOTE_SENSOR && sensor[i] -> id() == sh.lastSensorId())
        {
          param.text = "sensor is registered id: " + String(sensor[i] -> id()) + ", name: " + sensor[i] -> getName();
          break;
        }
      if (!param.text.length())
        param.text = "sensor was found id: " + String(sh.lastSensorId(),HEX);
    }
    gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param));
  }
}


void ArduinoFortress::parseGSMTaskResults(const GSMTask & task)
{
  if (task.isError())
    return;
  switch (task.task())
  {
  case GSMTask::GSM_TASK_READ_SMS:
    parseSMS(task.resultPhone().c_str(),task.resultText());
    break;
  default:
    break;
  }
}


void ArduinoFortress::parseSMS(const char * phone, const String & sms)
{
  GSMTask::GSM_SEND_SMS_T param;
  param.text.reserve(128);
  String sms_text = sms;
  memcpy(param.phone,phone,sizeof(param.phone));
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
      for (int i = 0; i < SENSOR_COUNT; i++)
        if (!sensor[i])
        {
          if (sh.setListenMode(true))
          {
            _listen_mode = true;
            memcpy(_listen_phone,phone,sizeof(_listen_phone));//saving phone for answer later
            param.text = "listen ok";
          }
          else
            param.text = "listen modem busy";  
          gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param));
          return;  
        }      
       param.text = "listen fullmemory error";
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
    if (sms_part[1] == "all")
    {
      param.text += "Free mem " + String(freeRam());
      for (int i = 0; i < SENSOR_COUNT; i++)
        if (sensor[i])
          param.text += String(", ") + sensor[i] -> report();
      gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param));
      return;
    }
    return;
  }
  if (sms_part[0] == "hello")
  {
    param.text = "Hello my dear friend!!! This is my first test sms";
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

void ArduinoFortress::printStackHeap()
{
    char * testVal;
    testVal = new char[100];
    Serial.print("Stack ");
    Serial.println((int)&testVal);
    Serial.print("Heap ");
    Serial.println((int)testVal);
    Serial.flush();
    delete[] testVal;
}


void ArduinoFortress::saveTestPhone()
{
  EEPROMManager::PHONE_ELEMENT_T phone;
  memset(&phone,0,sizeof(phone));
  const char ph[] = "+375290000000";
  memcpy(phone.number,ph,sizeof(ph));
  //memset(phone.number,0,sizeof(phone.number));
  EEPROMManager::save(EEPROMManager::EEPROM_PHONE_PART,1,(unsigned char *)&phone);
}
