#include "GSMModule.h"

GSMModule::GSMModule(HardwareSerial &serial) :
  _serial(&serial),
  _timeout_ms(10000),
  _buf_size(0),
  _r_flag(false)
{
  clearTasks();
  memset(_phone_numbers,0,sizeof(_phone_numbers));
}


void GSMModule::setPhone(unsigned char element, const char * phone_number)
{
  if (element >= PHONE_NUMBER_COUNT)
    return;
  strncpy(&_phone_numbers[element * PHONE_NUMBER_LENGTH], phone_number, PHONE_NUMBER_LENGTH);
  _phone_numbers[element * PHONE_NUMBER_LENGTH + PHONE_NUMBER_LENGTH - 1] = 0;//end of string
  Serial.write("GSM: register phone ");
  Serial.write(phone_number,PHONE_NUMBER_LENGTH);
  Serial.write("\n\r");
  Serial.flush();
}


void GSMModule::begin(unsigned long baud) 
{
  _serial->begin(baud);
}

void GSMModule::end() {
  _serial->end();
}

void GSMModule::send(const char *cmd) 
{
  _serial->write(cmd);
  _serial->write('\r');

#ifdef GSM_MODULE_DEBUG
  Serial.write("GSM OUT: ");
  Serial.println(cmd);
  Serial.flush();
#endif
}


bool GSMModule::addTask(const GSMTask & task)
{
  if ((task_queue.write_pointer + 1) % TASK_QUEUE_SIZE == task_queue.read_pointer)
    return false;//no space in queue
  task_queue.task[task_queue.write_pointer] = task;
  task_queue.task[task_queue.write_pointer].setCompleted(false);
  task_queue.task[task_queue.write_pointer].setError(false);
  task_queue.write_pointer = (task_queue.write_pointer + 1) % TASK_QUEUE_SIZE;
  return true;
}


void GSMModule::clearCurrentTask()
{
  task = GSMTask(GSMTask::GSM_TASK_NONE);
}


void GSMModule::clearTasks()
{
  for (int i = 0; i < TASK_QUEUE_SIZE; i++)
    task_queue.task[i] = GSMTask(GSMTask::GSM_TASK_NONE);
  task_queue.read_pointer = 0;
  task_queue.write_pointer = 0;
}

void GSMModule::proc() 
{
  while(_serial -> available())
  {
    _buf[_buf_size] = _serial -> read();
    //ignoring start \r and \n symbols
    if (!_buf_size && (_buf[_buf_size] == '\n' || _buf[_buf_size] == '\r'))
      continue;
    //Serial.write(_buf[_buf_size]);
    if ((_buf[_buf_size] == '\n' && _r_flag) || 
        (task.task() == GSMTask::GSM_TASK_SEND_SMS && (char)_buf[0] == '>'))//crutch for sms welcome mode
    {
      //parse string
      if (!parseSTD(_buf, _buf_size + 1))
        if (task.task() != GSMTask::GSM_TASK_NONE && !task.isCompleted())
          if (task.parseAnswer(_buf, _buf_size + 1))
          {
            if (!task.isCompleted() && task.isExtSend())//additional send required?
            {
              task.clearExtSend();
              send(task.gsmString().c_str());
            }
            
            if (task.isCompleted() && !task.isError() && task.task() == GSMTask::GSM_TASK_READ_SMS)
            {//check phone number of income sms. if it isn't at our white list, delete this task
              int i;
              for (i = 0; i < PHONE_NUMBER_COUNT; i++)
                if (String(&_phone_numbers[i * PHONE_NUMBER_LENGTH]).length() && String(&_phone_numbers[i * PHONE_NUMBER_LENGTH]) == task.resultPhone())
                  break;
              if (i == PHONE_NUMBER_COUNT)
              {
                clearCurrentTask();
#ifdef GSM_MODULE_DEBUG
                Serial.println("GSM: sms from non-white phone list. Banned");
                Serial.flush();
#endif
              }             
            }
          }
      _buf_size = 0;
      memset(_buf,0,sizeof(_buf));
      _r_flag = false;
      continue;
    }
    else
      if (_buf[_buf_size] == '\r')
        _r_flag = true;
    _buf_size++;
  }
  unsigned long current_time_ms = millis();
  if (task.task() != GSMTask::GSM_TASK_NONE && (current_time_ms <= _rcv_timeout && (current_time_ms - _rcv_timeout ) < 100000))
  {
    task.setCompleted(true);
    task.setError(true);
#ifdef GSM_MODULE_DEBUG
    Serial.println("GSM: gsm task execution timeout. No data received from gsm modem");
#endif
  }
  
  //if no task is active, get new one from queue
  if (task.task() == GSMTask::GSM_TASK_NONE && task_queue.read_pointer != task_queue.write_pointer)
  {
    task = task_queue.task[task_queue.read_pointer];
    task_queue.read_pointer = (task_queue.read_pointer + 1) % TASK_QUEUE_SIZE;
#ifdef GSM_MODULE_DEBUG
    Serial.println("GSM: starting new gsm task...");
    Serial.flush();
    _rcv_timeout = millis() + _timeout_ms;
#endif
    send(task.gsmString().c_str());
  }
}

typedef enum {
  GSM_PARSER_INCOME_SMS,
  GSM_PARSER_INCOME_RING 
} GSM_PARSER;

typedef struct _cmd_parser {
  String cmd;
  GSM_PARSER parser;
} CMD_PARSER;
#define PARSE_CMDS 2
const CMD_PARSER parser[PARSE_CMDS] = {
                              {"+CMTI: \"SM\",",GSM_PARSER_INCOME_SMS},
                              {"RING",GSM_PARSER_INCOME_RING}
};
bool GSMModule::parseSTD(byte * _buf, size_t size)
{
#ifdef GSM_MODULE_DEBUG
  Serial.write("GSM IN:");
  for (size_t i = 0; i < size; i++)
    Serial.write(_buf[i]);
    Serial.flush();
#endif
  //searching of standard initiative parser
  int parse_index;
  for (parse_index = 0; parse_index < PARSE_CMDS; parse_index++)
  {
    //Serial.write("compare length ");
    //Serial.println(parser[i].cmd.length(),DEC);
      
    int res = memcmp(parser[parse_index].cmd.c_str(),_buf,parser[parse_index].cmd.length());
    /*if(res != 0)
      Serial.println(res,DEC);
    else
      Serial.write("parse ok\r\n");*/
    if (!res)
      break;
  }

  if (parse_index == PARSE_CMDS)
    return false;//this is not a standard initiative modem command
            
  switch(parser[parse_index].parser)
  {
  case GSM_PARSER_INCOME_SMS:
  {
    String sms_in = String((char *)_buf + parser[parse_index].cmd.length());
    long sms_num = sms_in.toInt();
    if (sms_num)
    {
      GSMTask::GSM_READ_SMS_T task_data;
      task_data.number = sms_num;
      addTask(GSMTask(GSMTask::GSM_TASK_READ_SMS,&task_data));
    }
    break;
  }
  case GSM_PARSER_INCOME_RING:
    break;
  default:
    break;  
  }
  return true;
}


bool GSMModule::sendSMS(const String & text)
{
  GSMTask::GSM_SEND_SMS_T param;
  param.text = text;
    
  for (int i = 0; i < PHONE_NUMBER_COUNT; i++)
    if (String(&_phone_numbers[i * PHONE_NUMBER_COUNT]).length())
    {
#ifdef GSM_MODULE_DEBUG
      Serial.print("GSM: Sending sms to ");
      Serial.println(&_phone_numbers[i * PHONE_NUMBER_COUNT]);
      Serial.flush();
#endif
      memcpy(param.phone,&_phone_numbers[i * PHONE_NUMBER_COUNT],sizeof(param.phone));
      if (!addTask(GSMTask(GSMTask::GSM_TASK_SEND_SMS,&param)))
        return false;//no memory in queue
    }
  return true;      
}

