#include "GSMModule.h"

GSMModule::GSMModule(HardwareSerial &serial) :
  _serial(&serial),
  _first_time(1000),
  _intra_time(50),
  _overflow_size(0),
  _overflow_slot(0),
  _buf_size(0),
  _r_flag(false),
  task(GSMTask::GSM_TASK_NONE)
   {}


void GSMModule::setPhone(unsigned char element, const String & phone_number)
{
  if (element >= PHONE_NUMBER_COUNT)
    return;
  _phone_numbers[element] = phone_number;
  Serial.write("GSM: register phone ");
  Serial.write(phone_number.c_str());
  Serial.write("\n\r");
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
#endif
}


void GSMModule::setTimeout(long first_time, long intra_time) 
{
  _first_time = first_time;
  _intra_time = intra_time;
}

void GSMModule::addTask(const GSMTask & task)
{
  this -> task = task;
  this -> task.setCompleted(false);
  this -> task.setError(false);
  send(task.gsmString().c_str());
}


void GSMModule::clearCurrentTask()
{
  this -> task = GSMTask(GSMTask::GSM_TASK_NONE);
}


void GSMModule::clearTasks()
{
  
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
    if (_buf[_buf_size] == '\n' && _r_flag)
    {
      //parse string
      if (!parseSTD(_buf, _buf_size + 1))
        if (task.task() != GSMTask::GSM_TASK_NONE && !task.isCompleted())
          task.parseAnswer(_buf, _buf_size + 1);
      _buf_size = 0;
      _r_flag = false;
      continue;
    }
    else
      if (_buf[_buf_size] == '\r')
        _r_flag = true;
    _buf_size++;
  }

}

typedef enum {
  GSM_PARSER_INCOME_SMS,
  GSM_PARSER_INCOME_RING,
  GSM_PARSER_GET_SMS,
  GSM_PARSER_DELETE_SMS  
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
  Serial.write("GSM IN:");
  for (size_t i = 0; i < size; i++)
    Serial.write(_buf[i]);

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
    //Serial.println(sms_num,DEC);
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

/*
bool GSMModule::sendSMS(const String & number, const String & text)
{
  String sms_cmd;
  bool ok = false;
  //switch to text sms mode
  for (int i = 0; i < 2 && !ok; i++)
    ok = sendRecvUntil_P(G("AT+CMGF=1"), G("OK"));
  if (!ok)
    return false;
  //set sms encoding
  ok = false;
  for (int i = 0; i < 2 && !ok; i++)
    ok = sendRecvUntil_P(G("AT+CSCS= \"GSM\""), G("OK"));
  if (!ok)
    return false;
  //fill phone number 
  ok = false;
  sms_cmd = "AT+CMGS=\"" + number + "\"\r\n";
  for (int i = 0; i < 2 && !ok; i++)
    ok = sendRecvUntil_P(sms_cmd.c_str(), G(">"));    
  if (!ok)
    return false;
  //set sms text 
  ok = false;
  setTimeout(5000,0);
  sms_cmd = text + "\x1a";
  ok = sendRecvUntil_P(sms_cmd.c_str(), G("OK"));  
  setTimeout(1000,50); 
  //todo: if not ok, do escape from message mode 
  return ok;
}
*/
