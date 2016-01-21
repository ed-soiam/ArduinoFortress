#include "SHModem.h"
SHModem::SHModem(HardwareSerial &serial) :  _serial(&serial)
{
  _msg = 0;//pointer to NULL message
  _answer_machine = WAIT_ANSWER_NONE;
  _free = true;
  _error = false;
}

void SHModem::proc()
{
  Serial.print("SHModem proc()\n]r");
  //if (isFree())
}


//send command and wait for specific type of answer from modem
//returns false if port is busy or empty message
bool SHModem::sendCommand(const NetroMessage & msg,WAIT_ANSWER_T answer_machine, unsigned short timeot_ms)
{
  if (!isFree() || !msg.buffer() || !msg.size())
    return false;
  _answer_machine = answer_machine;
  _timeout = millis() + timeot_ms;
  //delete old message and create the new one
  if (_msg)
    delete _msg;
  _msg = new NetroMessage(msg);//create inner message object
  
  //stuffing data and send by one after other bytes
  unsigned char * buf = _msg -> buffer();
  for (unsigned char r_pointer = 0; r_pointer < _msg -> size(); r_pointer++)
  {
    while (!_serial -> availableForWrite());
    if ((buf[r_pointer] == INTERFACE_START_DATA && r_pointer) || buf[r_pointer] == INTERFACE_STAF_DATA)
      _serial -> write(INTERFACE_STAF_DATA);
    while (!_serial -> availableForWrite());
    _serial -> write(buf[r_pointer]);
  }
  return true;
}

