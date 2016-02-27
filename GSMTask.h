#ifndef _GSMTASK_H_
#define _GSMTASK_H_
#include "GSMModule.h"
class GSMTask {
public:
  friend class GSMModule;
  typedef enum{
    GSM_TASK_NONE = 0,
    GSM_TASK_SEND_SMS,
    GSM_TASK_READ_SMS,
    GSM_TASK_DELETE_SENT_SMS,
    GSM_TASK_DELETE_RECEIVED_SMS,
    GSM_TASK_GETTIME    
  } GSM_TASK_T;

  typedef struct _gsm_send_sms {
    String phone;
    String text;
  } GSM_SEND_SMS_T;

  typedef struct _gsm_read_sms {
    long number;
  } GSM_READ_SMS_T;
  
  GSMTask(GSM_TASK_T task,void * data = NULL);
  GSMTask(const GSMTask & task);
  GSMTask & operator=(const GSMTask & task);
  //get current task
  GSM_TASK_T task() const {return _task;}
  
  //get task complete state
  bool isCompleted() const {return _completed;}
  
  //get task error
  bool isError() const {return _error;}

  const void * resultData() const {return 0;}
  
protected:
  //Friends only functions
  bool setCompleted(bool value) {_completed = value;}
  bool setError(bool value) {_error = value;}
  bool parseAnswer(byte * _buf, size_t size);
  //get output gsm string for gsmmoddule friend class
  String gsmString() const {return _gsm_string;}
private:
  typedef enum {
    PARSE_NONE,
    PARSE_ECHO,
    PARSE_IN_SMS_INFO,
    PARSE_IN_SMS_TEXT,
    PARSE_OK
  } PARSE_STAGE_T;
  PARSE_STAGE_T parse_stage;
  GSM_TASK_T _task;
  bool _completed;
  bool _error;
  String _phone_number;
  String _text;
  long _tmp_number;
  String _gsm_string;
};
#endif
