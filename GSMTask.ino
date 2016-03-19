#include "GSMTask.h"

GSMTask::GSMTask(GSM_TASK_T task,void * data)
{  
  _gsm_string.reserve(32);
  _phone_number.reserve(PHONE_NUMBER_LENGTH);
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
  this -> _gsm_string.reserve(32);
  this -> _phone_number.reserve(PHONE_NUMBER_LENGTH);
  this -> _task = task._task;
  this -> _completed = task._completed;
  this -> _error = task._error;
  this -> _phone_number = task._phone_number;
  this ->  _text = task._text;
  this -> _tmp_number = task._tmp_number;
  this -> _bool_value = task._bool_value;
  this -> _gsm_string = task._gsm_string;
  this -> _parse_stage = task._parse_stage;
  this -> _ext_send = task._ext_send;
  return * this;
}


void GSMTask::_setTask(GSM_TASK_T task,void * data)
{
  _task = task;
  _completed = false;
  _error = false;
  _ext_send = false;
  //create output string to gsm
  switch (_task)
  {
  case GSM_TASK_NONE:
    _completed = false;
    _error = true;
    break;
    
  case GSM_TASK_SET_SMS_MODE:
    _gsm_string = "AT+CMGF=1";
    break;
    
  case GSM_TASK_SET_GSM_ENCODING:
    _gsm_string = "AT+CSCS= \"GSM\"";
    break;
    
  case GSM_TASK_SEND_SMS:
  {
    if (!data)
    {
      _completed = false;
      _error = true;
      break;
    }
    GSM_SEND_SMS_T * sms = (GSM_SEND_SMS_T *)data;
    _phone_number = sms -> phone;
    _text = sms -> text;
    _gsm_string = "AT+CMGS=\"" + _phone_number + "\"";
    break;
  }
    
  case GSM_TASK_READ_SMS:
    _gsm_string = "AT+CMGR=";
    _gsm_string.concat(((GSM_READ_SMS_T *)data) -> number);
    break;
    
  case GSM_TASK_DELETE_ALL_SMS:
    //_gsm_string = "AT+CMGDA=\"DEL ALL\"";
    _gsm_string = "AT+CMGD=0,4";
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
  if (_ext_send)
    return false;
    
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
      Serial.flush();
#endif
      return false;
    }
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Echo was caught");
      Serial.flush();
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
    case GSM_TASK_SEND_SMS:
      _parse_stage = PARSE_SMS_MODE_WELCOME;
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
  {
    String cmgr_str = "+CMGR:";
    String ok_str = "OK\r\n";
    int res = memcmp(cmgr_str.c_str(),_buf,cmgr_str.length());
    if (!res)
    {
      int i,find_quotes = 0;
      //parse something like that, get phone number
      //+CMGR: "REC UNREAD","+375290000000","","16/03/06,13:45:30+12"
      _phone_number = "";
      for (i = 0; i < size; i++)
      {
        if ((char)_buf[i] == '\"')
          find_quotes++;
        else
        {
          if (find_quotes >= 4)
            break;
          if (find_quotes == 3)
            _phone_number.concat((char)_buf[i]);
        }
      }
      _parse_stage = PARSE_IN_SMS_TEXT;
      return true;
    }
    res = memcmp(ok_str.c_str(),_buf,ok_str.length());
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: End of command(OK) was found. No sms was found.");
      Serial.flush();
#endif
      _completed = true;
      _error = true;
      return true;
    }
#ifdef GSM_TASKS_DEBUG
    Serial.println("GSMTask: Not a sms info");
    Serial.flush();
#endif
    return false;
  }
    
  case PARSE_IN_SMS_TEXT:
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: SMS text was caught");
      Serial.flush();
#endif
    _text = String((char *)_buf);
    _parse_stage = PARSE_OK;
    return true;
    
  case PARSE_SMS_MODE_WELCOME:
    if ((char)_buf[0] != '>')
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: No welcome to text mode was found");
      Serial.flush();
#endif
      return false;      
    }
    _gsm_string = _text + "\x1a";
    _ext_send = true;
    _parse_stage = PARSE_SMS_SEND_STATE;
    return true;
    
  case PARSE_SMS_SEND_STATE:
  {
    String cmgs_str = "+CMGS:";
    int res = memcmp(cmgs_str.c_str(),_buf,cmgs_str.length()); 
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: SMS send process is on.");
      Serial.flush();
#endif
      _parse_stage = PARSE_OK;
      return true;
    }
#ifdef GSM_TASKS_DEBUG
    Serial.println("GSMTask: No SMS send process was found.");
    Serial.flush();
#endif
    return false;
  }
  
  case  PARSE_OK:
  {
    String ok_str = "OK\r\n";
    String err_str = "ERROR\r\n";
    int res = memcmp(ok_str.c_str(),_buf,ok_str.length());
    if (!res)
    {
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: End of command(OK) was found");
      Serial.flush();
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
      Serial.flush();
#endif
      _completed = true;
      _error = true;
      return true;
    }    
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Not an end of command");
      Serial.flush();
#endif
      return false;
  }
  
  default:
#ifdef GSM_TASKS_DEBUG
      Serial.println("GSMTask: Unknown parse stage");
      Serial.flush();
#endif
    return false;
  }

  return false;
}

