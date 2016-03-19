#ifndef _GSMTASK_H_
#define _GSMTASK_H_
#include "GSMModule.h"
#define IMEI_LENGTH 14
class GSMTask {
public:
  friend class GSMModule;
  typedef enum{
    GSM_TASK_NONE = 0,
    GSM_TASK_SET_SMS_MODE,
    GSM_TASK_SET_GSM_ENCODING,
    GSM_TASK_SEND_SMS,
    GSM_TASK_READ_SMS,//returns resultText() and resultPhone()
    GSM_TASK_DELETE_ALL_SMS,
    GSM_TASK_DELETE_SENT_SMS,
    GSM_TASK_DELETE_READ_SMS,
    GSM_TASK_DELETE_UNREAD_SMS,
    GSM_TASK_GET_TIME,
    GSM_TASK_GET_REGISTERED,//returns resultBool()
    GSM_TASK_GET_ATTACHED,//returns resultBool()
    GSM_TASK_GET_IMEI//returns resultText()    
  } GSM_TASK_T;

  typedef struct _gsm_send_sms {    
    String text;
    char phone[PHONE_NUMBER_LENGTH];
  } GSM_SEND_SMS_T;

  typedef struct _gsm_read_sms {
    long number;
  } GSM_READ_SMS_T;
  
  GSMTask(GSM_TASK_T task = GSM_TASK_NONE,void * data = NULL);
  GSMTask(const GSMTask & task);
  GSMTask & operator=(const GSMTask & task);
  //get current task
  GSM_TASK_T task() const {return _task;}
  
  //get task complete state
  bool isCompleted() const {return _completed;}
  
  //get task error
  bool isError() const {return _error;}

  //results for user
  bool resultBool() const {return _bool_value;}
  String resultPhone() const {return _phone_number;}
  String resultText() const {return _text;}
  long resultInt() const {return _tmp_number;}
protected:
  /*Friends only functions*/
  bool setCompleted(bool value) {_completed = value;}
  bool setError(bool value) {_error = value;}
  bool parseAnswer(byte * _buf, size_t size);
  //Is additional send of subcommand is requied?
  bool isExtSend() const {return _ext_send;}
  //Clear additional send flag
  void clearExtSend() {_ext_send = false;}
  
  //get output gsm string for gsmmoddule friend class
  String gsmString() const {return _gsm_string;}
private:
  typedef enum {
    PARSE_NONE,
    PARSE_ECHO,
    PARSE_IN_SMS_INFO,
    PARSE_IN_SMS_TEXT,
    PARSE_ATTACHED_STATE,
    PARSE_REGISTERED_STATE,
    PARSE_SMS_MODE_WELCOME,
    PARSE_SMS_SEND_STATE,
    PARSE_OK
  } PARSE_STAGE_T;
  PARSE_STAGE_T _parse_stage;
  GSM_TASK_T _task;
  inline void _setTask(GSM_TASK_T task,void * data);
  bool _ext_send;//additional send of subcommand in main command during parse process
  bool _completed;
  bool _error;
  String _gsm_string;

  //returned to user values
  String _phone_number;
  String _text;
  long _tmp_number;
  bool _bool_value;
};
#endif
