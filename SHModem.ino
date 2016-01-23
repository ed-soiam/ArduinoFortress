#include "SHModem.h"
SHModem::SHModem(HardwareSerial &serial) :  _serial(&serial)
{
  _msg = 0;//pointer to NULL message
  _free = true;
  _error = false;
  clearRX();
  _packages = 0;
}

void SHModem::begin(unsigned long baud) 
{
  _serial -> begin(baud);
}

void SHModem::clearRX()
{
  memset(_rcv_buf,0,sizeof(_rcv_buf));
  _staff = 0;
  _rcv_byte_num = 0;
}


void SHModem::proc()
{
  unsigned char data_byte;
  while (_serial -> available())
  {
    data_byte = (unsigned char)_serial->read();
                    
    //catching of the first byte?
    if(!_rcv_byte_num)
    {
      _staff = 0;
      //sync error
      if (data_byte != INTERFACE_START_DATA)
      {
        Serial.println("SH in: error start of message");
        continue;
      }
    }
    else
    {
      //staff analyzing
      if ((data_byte == INTERFACE_STAF_DATA) && (!_staff))
      {
        _staff = 1;
        continue;
      }
    }
    //caught of staff?
    if (_staff)
    {
      _staff = 0;//clear staff
      // if after staff byte we didn't catch INTERFACE_START_DATA or INTERFACE_STAF_DATA
      if (data_byte != INTERFACE_START_DATA && data_byte != INTERFACE_STAF_DATA)
      {
        clearRX();
        continue;
      }
    }
    else
      if (data_byte == INTERFACE_START_DATA && _rcv_byte_num)//if no staffing and we caught START_BYTE
        clearRX();
    
    //rx buffer overflow protection
    if (_rcv_byte_num < INTERFACE_EXT_PACKET_LENGTH)
      _rcv_buf[_rcv_byte_num++] = data_byte;

    //the end of the command?
    if (_rcv_byte_num >= (INTERFACE_SIZE_OFFSET + 1) && _rcv_byte_num >= (INTERFACE_SIZE_OFFSET + 1) + _rcv_buf[INTERFACE_SIZE_OFFSET] + 2)
    {
            _packages++;
            parseRXCommand();
            Serial.print("SH in: ");
            for (unsigned char i = 0; i < _rcv_byte_num; i++)
            {
              Serial.print(_rcv_buf[i],HEX);
              Serial.print(" ");
            }
            Serial.print("\n\r");
            clearRX();
    } 
  }
  stateMachine();
}


void SHModem::stateMachine()
{
  unsigned long current_time = millis();
  //timeout and timeoverflow protection(100sec)
  if (!isFree() && (_timeout < current_time && (current_time - _timeout ) < 100000))
  {
    _free = true;
    _error = true;
  }
}


//send command and wait for specific type of answer from modem
//returns false if port is busy or empty message
bool SHModem::sendCommand(const NetroMessage & msg, unsigned short timeot_ms)
{
  if (!isFree() || !msg.buffer() || !msg.size())
    return false;
  _free = false;
  _timeout = millis() + timeot_ms;
  //delete old message and create the new one
  if (_msg)
    delete _msg;
  _msg = new NetroMessage(msg);//create inner message object
  Serial.print("SH OUT:");
  //stuffing data and send by one after other bytes
  unsigned char * buf = _msg -> buffer();
  for (unsigned char r_pointer = 0; r_pointer < _msg -> size(); r_pointer++)
  {
    while (!_serial -> availableForWrite());
    if ((buf[r_pointer] == INTERFACE_START_DATA && r_pointer) || buf[r_pointer] == INTERFACE_STAF_DATA)
    {
      Serial.print(INTERFACE_STAF_DATA,HEX);
      _serial -> write(INTERFACE_STAF_DATA);
      while (!_serial -> availableForWrite());
    }
    Serial.print(buf[r_pointer],HEX);
    Serial.print(" "); 
    _serial -> write(buf[r_pointer]);
    
  }
  Serial.print("\n\r"); 
  return true;
}


void SHModem::parseRXCommand()
{
  
}

