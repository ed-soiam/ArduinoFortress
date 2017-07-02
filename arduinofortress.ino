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
  memset(_listen_phone, 0, sizeof(_listen_phone));
  //saveTestPhone();
  //load phone settings from eeprom
  for (unsigned char i = 0; i < PHONE_NUMBER_COUNT; i++)
  {
    EEPROMManager::PHONE_ELEMENT_T phone;
    if (!EEPROMManager::load(EEPROMManager::EEPROM_PHONE_PART, i, (unsigned char *)&phone))
      Serial.println("No EEPROM was found to load from");
    if (phone.number[0] != 0xff && phone.number[0] != 0x00)
      gsm.setPhone(i, phone.number);
  }
  //load sensors info from eeprom
  for (unsigned char i = 0; i < SENSOR_COUNT; i++)
  {
    String load_report_str;
    EEPROMManager::SENSOR_ELEMENT_T raw_sensor;
    EEPROMManager::load(EEPROMManager::EEPROM_SENSOR_PART, i, (unsigned char *)&raw_sensor);
    //factory needed :)
    switch (raw_sensor.type)
    {
      case Sensor::ANALOG_SENSOR:
        sensor[i] = new AnalogSensor(&raw_sensor);
        sensor[i] -> setEnabled(false);
        load_report_str = String("AnalogSensor '") + sensor[i] -> getName() + String("' in cell ") + String(i);
        Serial.println(load_report_str);
        Serial.flush();
        break;
      case Sensor::DIGITAL_SENSOR:
      case Sensor::I2C_SENSOR:
        sensor[i] = NULL;
        break;
      case Sensor::REMOTE_SENSOR:
        sensor[i] = new RemoteSensor(&raw_sensor);
        load_report_str = String("RemoteSensor '") + sensor[i] -> getName() + String("' in cell ") + String(i);
        Serial.println(load_report_str);
        Serial.flush();
        break;
      default:
        if (i == 0)//create analog voltage sensor if no sensor is presented in cell 0
        {
          Serial.println("Creating new Power voltage sensor");
          sensor[i] = new AnalogSensor("Power voltage", 0, 450, true); //11.2v low on 5v power supply
          sensor[i] -> toEEPROMData(&raw_sensor);
          sensor[i] -> setEnabled(false);
          EEPROMManager::save(EEPROMManager::EEPROM_SENSOR_PART, i, (unsigned char *)&raw_sensor);
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
  _alarm_sys = new AlarmSystem(sensor,SENSOR_COUNT);
  gsm.begin(115200);
  sh.begin(115200);
  GSMTask task;
  //clearing SIM module memory from all sms
  task = GSMTask(GSMTask::GSM_TASK_DELETE_ALL_SMS, 0);
  gsm.addTask(task);
  //prepare SIM module text mode
  task = GSMTask(GSMTask::GSM_TASK_SET_SMS_MODE, 0);
  gsm.addTask(task);
  task = GSMTask(GSMTask::GSM_TASK_SET_GSM_ENCODING, 0);
  gsm.addTask(task);

}


void ArduinoFortress::proc()
{
  //checking alarming system
  String sms = _alarm_sys -> alarmSMS();
  if (sms.length())
    gsm.sendSMS(sms); 

  //gsm
  gsm.proc();
  if (gsm.currentTask().isCompleted())
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
    memcpy(param.phone, _listen_phone, sizeof(param.phone));
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
          param.text = "sensor has been already saved, id: " + String(sensor[i] -> id(), HEX) + ", name: " + sensor[i] -> getName();
          break;
        }
      if (!param.text.length())
        param.text = "sensor was found id: " + String(sh.lastSensorId(), HEX);
    }
    gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS, &param));
  }
  
  unsigned long id = sh.lastAlarmId();
  if (id != (unsigned long)(-1))
    this -> alarm(id);
}


void ArduinoFortress::alarm(unsigned long id)
{
  Serial.print("Remote alarm ");
  Serial.println(id,HEX);
  for (int i = 0; i < SENSOR_COUNT; i++)
    if (sensor[i] && sensor[i] -> sensorType() == Sensor::REMOTE_SENSOR && sensor[i] -> id() == id)
      sensor[i] -> setAlarm(true);
}


void ArduinoFortress::parseGSMTaskResults(const GSMTask & task)
{
  if (task.isError())
    return;
  switch (task.task())
  {
    case GSMTask::GSM_TASK_READ_SMS:
      parseSMS(task.resultPhone().c_str(), task.resultText());
      break;
    default:
      break;
  }
}

typedef enum {
  SMS_SENSOR = 0,
  SMS_PHONE,
  SMS_MEMORY,
  SMS_REPORT,
  SMS_HELLO,
  SMS_ALARM,
  SMS_LAST //used for size
} SMS_T;

typedef struct _sms_parser {
  char cmd[10];
  SMS_T parser;
} SMS_PARSER;
const SMS_PARSER sms_parser[SMS_LAST] = {
  {"sensor", SMS_SENSOR},
  {"phone", SMS_PHONE},
  {"memory", SMS_MEMORY},
  {"report", SMS_REPORT},
  {"hello", SMS_HELLO},
  {"alarm", SMS_ALARM}
};


void ArduinoFortress::parseSMS(const char * phone, const String & sms)
{
  GSMTask::GSM_SEND_SMS_T param;
  param.text.reserve(128);
  param.text = "";
  String sms_text = sms;
  memcpy(param.phone, phone, sizeof(param.phone));
  sms_text.toLowerCase();
  //removing end symbols
  while (sms_text.charAt(sms_text.length() - 1) == '\n' || sms_text.charAt(sms_text.length() - 1) == '\r')
    sms_text.remove(sms_text.length() - 1);

  String sms_part[6];//cmd,subcmd and 6 parameters
  int start_substring = 0;
  bool any_symbols = false;
  int part_number = 0;
  for (int j = 0; j < sms.length(); j++)
    if (sms_text.charAt(j) == ' ' || j == sms.length() - 1)
    {
      if (any_symbols)
      {
        sms_part[part_number] = sms_text.substring(start_substring, j);
        part_number++;
        if (part_number >= 6)
          break;//too many words
      }
      start_substring = j + 1;
      any_symbols = false;
    }
    else
      any_symbols = true;

  //searching for parser to received command
  SMS_T parser_index = SMS_LAST;
  for (int i = 0; i < SMS_LAST; i++)
    if (sms_part[0] == sms_parser[i].cmd)
    {
      parser_index = sms_parser[i].parser;
      break;
    }
  if (parser_index == SMS_LAST)
  {
    Serial.println(sms_part[0].c_str());
    return;//unknown command, no parser
  }

  //parse subcommands
  switch (parser_index)
  {
  case SMS_SENSOR:
  {
    if (sms_part[1] == "listen")
    {
      if (_listen_mode)
      {
        param.text = "listen already in progress";
        break;
      }

      for (int i = 0; i < SENSOR_COUNT; i++)
        if (!sensor[i])
        {
          if (sh.setListenMode(true))
          {
            _listen_mode = true;
            memcpy(_listen_phone, phone, sizeof(_listen_phone)); //saving phone for answer later
            param.text = "listen ok";
          }
          else
            param.text = "listen modem busy";
          break;
        }
      if (!param.text.length())
        param.text = "listen fullmemory error";
      break;
    }
    
    if (sms_part[1] == "save")
    {
      EEPROMManager::SENSOR_ELEMENT_T raw_sensor;
      if (sms_part[2] == "last")
      {
        if (sh.lastSensorId() == (unsigned long)(-1))
        {
          param.text = "sensor invalid id error";
          break;
        }
        //check for already saved sensor
        for (int i = 0; i < SENSOR_COUNT; i++)
          if (sensor[i] && sensor[i] -> sensorType() == Sensor::REMOTE_SENSOR && sensor[i] -> id() == sh.lastSensorId())
          {
            param.text = "sensor has been already saved id: " + String(sensor[i] -> id(), HEX) + ", name: " + sensor[i] -> getName();
            break;
          }
        if (param.text.length())
          break;
        for (int i = 0; i < SENSOR_COUNT; i++)
          if (!sensor[i])
          {
            Serial.print("Creating new Remote sensor ");
            Serial.println(sms_part[3]);
            sensor[i] = new RemoteSensor(sms_part[3].c_str(), sh.lastSensorId());
            sensor[i] -> toEEPROMData(&raw_sensor);
            EEPROMManager::save(EEPROMManager::EEPROM_SENSOR_PART, i, (unsigned char *)&raw_sensor);
            param.text = "sensor saved ok";
            break;
          }
        if (!param.text.length())
          param.text = "save fullmemory error";
        break;
      }
      param.text = "save param error";
      break;
    }
    param.text = "listen subcommand error";
    break;
  }

  case SMS_PHONE:
  {
    return;
  }

  case SMS_MEMORY:
  {
    return;
  }

  case SMS_REPORT:
  {
    if (sms_part[1] == "all")
    {
      param.text.concat("Free mem ");
      param.text.concat(freeRam());
      for (int i = 0; i < SENSOR_COUNT; i++)
        if (sensor[i])
        {
          param.text.concat(", ");
          param.text.concat(sensor[i] -> report());
        }
      break;
    }
    param.text = "report subcommand error";
    break;
  }

  case SMS_HELLO:
  {
    param.text = "Hello my dear friend!!! I am online";
    break;
  }

  case SMS_ALARM:
  {
    return;
  }
  }
  gsm.addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS, &param));
}


int ArduinoFortress::freeRam()
{
#if defined (__AVR_ATmega128__)
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
#else
  return 0;
#endif
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
  memset(&phone, 0, sizeof(phone));
  const char ph[] = "+375290000000";
  memcpy(phone.number, ph, sizeof(ph));
  //memset(phone.number,0,sizeof(phone.number));
  EEPROMManager::save(EEPROMManager::EEPROM_PHONE_PART, 1, (unsigned char *)&phone);
}
