#ifndef _GSMMODULE_H_
#define _GSMMODULE_H_
#include "defs.h"
#include "GSMTask.h"
#define GSM_BUFFER_SIZE 256
#define GSM_MAX_CALLBACK 3
#define TASK_QUEUE_SIZE 8
class GSMModule {
public:
	GSMModule(HardwareSerial &serial);

  void setPhone(unsigned char element, const char * phone_number);
	void begin(unsigned long baud);
	void end();
 
  //add new gsm task to module(put it in run queue)
  bool addTask(const GSMTask & task);
  
  //get current executing GSM task
  GSMTask currentTask() const {return task;}
  
	//delete current task and execute next if exists
	void clearCurrentTask();
  
  //delete all GSM tasks in queue,except current
  void clearTasks();
  
	// Set timeout for GSM task, ms;
	void setTimeout(unsigned long value = 10000){_timeout_ms = value;}
  unsigned long timeout() const {return _timeout_ms;}
  
	// Must be called frequently to check incoming data
	void proc();
  
  //Send sms to all phones in white list
  bool sendSMS(const String & text);
  
	inline HardwareSerial &serial() { return *_serial; }

private:

  typedef struct _task_queue_t 
  {
    GSMTask task[TASK_QUEUE_SIZE];
    int read_pointer;
    int write_pointer;
  } TASK_QUEUE_T;
  
  // Send AT command.  Don't use '\r' as command suffix.
  void send(const char *cmd);
  //parse standard income initiative string from modem
  bool parseSTD(byte * _buf, size_t size);
  
	HardwareSerial *_serial;
  char _phone_numbers[PHONE_NUMBER_COUNT * PHONE_NUMBER_LENGTH];
	byte _buf[GSM_BUFFER_SIZE];
	size_t _buf_size;
	unsigned long _timeout_ms;
  bool _r_flag;//'\r' was caught in last byte
  GSMTask task;
  TASK_QUEUE_T task_queue; 
  unsigned long _rcv_timeout;
};

#endif
