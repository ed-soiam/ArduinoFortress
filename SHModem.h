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
  bool sendCommand(const NetroMessage & msg, unsigned short timeot_ms = 200);
  //last caught sensor id in listen to new sensors mode
  unsigned long lastSensorId() const{return _sensor_id;} 
  //last sensor id, which sent us an alarm
  unsigned long lastAlarmId() const{return (unsigned long)(-1);} 
  
  bool setListenMode(bool value);
  bool isListenMode() const {return _listening;}
private:
  void clearSensorId(){_sensor_id = (unsigned long)(-1);}
  void getSensorId();
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
  
  //for listening mode
  unsigned long _sensor_id;
  bool _listening;
  unsigned long _listen_timeout;
  unsigned long _last_command_time; 
};
#endif
