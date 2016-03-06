#include "GSMTask.h"

GSMTask::GSMTask(GSM_TASK_T task,void * data)
{  
  _tmp_number = 0;
  _bool_value = false;
  _parse_stage = PARSE_NONE;
  _setTask(task,data);  
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
  this -> _bool_value = task._bool_value;
  this -> _gsm_string = task._gsm_string;
  this -> _parse_stage = task._parse_stage;
  return * this;
}


void GSMTask::_setTask(GSM_TASK_T task,void * data)
{
  _task = task;
  _completed = false;
  _error = false;
  //create output string to gsm
  switch (_task)
  {
  case GSM_TASK_NONE:
    _completed = false;
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
    
  case GSM_TASK_DELETE_READ_SMS:
    _gsm_string = "AT+CMGDA=\"DEL READ\"";
    break;

  case GSM_TASK_DELETE_UNREAD_SMS:
    _gsm_string = "AT+CMGDA=\"DEL UNREAD\"";
    break;
    
  case GSM_TASK_GET_TIME:
    _completed = true;
    _error = true; 
    break;
    
  case GSM_TASK_GET_REGISTERED:
    _gsm_string = "AT+CREG?";
    break;
      
  case GSM_TASK_GET_ATTACHED:
    _gsm_string = "AT+CGATT?";
    break;
    
  case GSM_TASK_GET_IMEI:
    _gsm_string = "AT+GSN";
    break;  
  }
  _parse_stage = PARSE_ECHO;
}


bool GSMTask::parseAnswer(byte * _buf, size_t size)
{
  switch (_parse_stage)
  {
  case PARSE_NONE:
    return false;
    
  case PARSE_ECHO:
  {
    int res = memcmp(_gsm_string.c_str(),_buf,_gsm_string.length());
    if (res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.print("GSMTask: Not an echo, differs at ");
      Serial.println(res);
#endif
      return false;
    }
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Echo was caught");
#endif
    switch (_task)
    {
    case GSM_TASK_READ_SMS:
      _parse_stage = PARSE_IN_SMS_INFO;
      return true;
    case GSM_TASK_GET_ATTACHED:
      _parse_stage = PARSE_ATTACHED_STATE;
      return true;
    case GSM_TASK_GET_REGISTERED:
      _parse_stage = PARSE_REGISTERED_STATE;
      return true;
    default: 
      _parse_stage = PARSE_OK;
      return true;
    }
    break;
  }
  
  case PARSE_ATTACHED_STATE:
  {
    String yes_str = "+CGATT: 1";
    String no_str = "+CGATT: 0";
    int res = memcmp(yes_str.c_str(),_buf,yes_str.length());
    if (!res)
    {
      _bool_value = true;
      _parse_stage = PARSE_OK;
      return true;
    }
    res = memcmp(no_str.c_str(),_buf,no_str.length());
    if (!res)
    {
      _bool_value = false;
      _parse_stage = PARSE_OK;
      return true;
    }
    return false;
  }
  
  case PARSE_REGISTERED_STATE:
  {
    String yes_str = "+CREG: 0,1";
    String no_str = "+CREG: ";
    int res = memcmp(yes_str.c_str(),_buf,yes_str.length());
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Resistered = 1");
#endif
      _bool_value = true;
      _parse_stage = PARSE_OK;
      return true;
    }
    res = memcmp(no_str.c_str(),_buf,no_str.length());
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Resistered = 0");
#endif
      _bool_value = false;
      _parse_stage = PARSE_OK;
      return true;
    }
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Not a registered state");
#endif
    return false;
  }
  
  case PARSE_IN_SMS_INFO:
    break;//TODO:
    
  case PARSE_IN_SMS_TEXT:
    break;//TODO:
    
  case  PARSE_OK:
  {
    String ok_str = "OK\r\n";
    String err_str = "OK\r\n";
    int res = memcmp(ok_str.c_str(),_buf,ok_str.length());
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: End of command(OK) was found");
#endif
      _completed = true;
      _error = false;
      return true;
    }
    res = memcmp(err_str.c_str(),_buf,err_str.length());
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: End of command(ERROR) was found");
#endif
      _completed = true;
      _error = true;
      return true;
    }    
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Not an end of command");
#endif
      return false;
  }
  
  default:
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Unknown parse stage");
#endif
    return false;
  }
}

