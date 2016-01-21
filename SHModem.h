#ifndef _SHMODEM_H_
#define _SHMODEM_H_
#include "NetroMessage.h"
class SHModem {
public:
  typedef enum {
    WAIT_ANSWER_NONE,
    WAIT_ANSWER_STANDARD,
    WAIT_ANSWER_PARAM,
    WAIT_ANSWER_LISTEN_ID,
    
  } WAIT_ANSWER_T;
  SHModem(HardwareSerial &serial);
  // Must be called frequently to check incoming data
  void proc();
  //is port free of transaction
  bool isFree() const {return _free;}
  //is last transaction errorous?
  bool isError() const {return _error;}
  //send command and wait for specific type of answer from modem
  //returns false if port is busy or empty message
  bool sendCommand(const NetroMessage & msg,WAIT_ANSWER_T answer_machine, unsigned short timeot_ms = 100);
private:
  HardwareSerial *_serial;
  NetroMessage * _msg;
  unsigned long _timeout;
  WAIT_ANSWER_T _answer_machine;
  bool _free;
  bool _error;
  //String s;
};
#endif
