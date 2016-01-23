#ifndef _SHMODEM_H_
#define _SHMODEM_H_
#include "NetroMessage.h"
class SHModem {
public:

  SHModem(HardwareSerial &serial);
  void begin(unsigned long baud); 
  // Must be called frequently to check incoming data
  void proc();
  //is port free of transaction
  bool isFree() const {return _free;}
  //is last transaction errorous?
  bool isError() const {return _error;}
  //send command and wait for specific type of answer from modem
  //returns false if port is busy or empty message
  bool sendCommand(const NetroMessage & msg, unsigned short timeot_ms = 100);
  
private:
  void clearRX();
  //logic modem machine
  void stateMachine();

  void parseRXCommand();
  
  HardwareSerial *_serial;
  NetroMessage * _msg;
  unsigned long _timeout;
  bool _free;
  bool _error;
  //for incoming answers and commands
  unsigned long _packages;//rx packages after object created
  unsigned char _rcv_buf[INTERFACE_EXT_PACKET_LENGTH];
  bool _staff;
  unsigned char _rcv_byte_num;

};
#endif
