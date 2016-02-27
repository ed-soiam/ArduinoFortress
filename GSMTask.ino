#include "GSMTask.h"

GSMTask::GSMTask(GSM_TASK_T task,void * data)
{
  _task = task;
  _completed = false;
  _error = false;
  _tmp_number = 0;
  parse_stage = PARSE_NONE;
  //create output string to gsm
  switch (_task)
  {
  case GSM_TASK_NONE:
    _completed = true;
    _error = true;
    break;
  case GSM_TASK_SEND_SMS:
    break;
    
  case GSM_TASK_READ_SMS:
    _gsm_string = "AT+CMGR=";
    _gsm_string.concat(((GSM_READ_SMS_T *)data) -> number);
    break;
    
  case GSM_TASK_DELETE_SENT_SMS:
    _gsm_string = "AT+CMGDA=\"DEL SENT\"";
    break;
    
  case GSM_TASK_DELETE_RECEIVED_SMS:
    _gsm_string = "AT+CMGDA=\"DEL READ\"";
    break;
    
  case GSM_TASK_GETTIME:
    _completed = true;
    _error = true; 
    break;
       
  }
  parse_stage = PARSE_ECHO;
}


GSMTask::GSMTask(const GSMTask & task)
{
  *this = task;
}


GSMTask &GSMTask::operator=(const GSMTask & task)
{
  this -> _task = task._task;
  this -> _completed = task._completed;
  this -> _error = task._error;
  this -> _phone_number = task._phone_number;
  this ->  _text = task._text;
  this -> _tmp_number = task._tmp_number;
  this -> _gsm_string = task._gsm_string;
  return * this;
}

bool GSMTask::parseAnswer(byte * _buf, size_t size)
{
  switch (parse_stage)
  {
  case PARSE_NONE:
    return false;
  case PARSE_ECHO:
  {
    int res = memcmp(_gsm_string.c_str(),_buf,_gsm_string.length());
    break;
  }  
  }
}

