#include "defs.h"
#include "SHModem.h"

SHModem::SHModem(HardwareSerial &serial) :  _serial(&serial)
{
  _msg = 0;//pointer to NULL message
  _free = true;
  _error = false;
  _listening = false;
  clearRX();
  _packages = 0;
  clearSensorId();
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
#ifdef SH_TRANSPORT_DEBUG
            Serial.print("SH IN: ");
            for (unsigned char i = 0; i < _rcv_byte_num; i++)
            {
              Serial.print(_rcv_buf[i],HEX);
              Serial.print(" ");
            }
            Serial.print("\n\r");
#endif
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
    _listening = false;
  }

  //timeout and timeoverflow protection(100sec)
  if (_listening && _free)
  { 
    if (_listen_timeout < current_time && (current_time - _timeout ) < 100000)
      _listening = false;
    else
      getSensorId();
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
#ifdef SH_TRANSPORT_DEBUG
  Serial.print("SH OUT:");
#endif
  //stuffing data and send by one after other bytes
  unsigned char * buf = _msg -> buffer();
  for (unsigned char r_pointer = 0; r_pointer < _msg -> size(); r_pointer++)
  {
    while (!_serial -> availableForWrite());
    if ((buf[r_pointer] == INTERFACE_START_DATA && r_pointer) || buf[r_pointer] == INTERFACE_STAF_DATA)
    {
#ifdef SH_TRANSPORT_DEBUG
      Serial.print(INTERFACE_STAF_DATA,HEX);
#endif
      _serial -> write(INTERFACE_STAF_DATA);
      while (!_serial -> availableForWrite());
    }
#ifdef SH_TRANSPORT_DEBUG
    Serial.print(buf[r_pointer],HEX);
    Serial.print(" "); 
#endif
    _serial -> write(buf[r_pointer]);
    
  }
  Serial.print("\n\r"); 
  return true;
}


typedef enum 
{
  RX_PARSER_STD_ANSWER,
  RX_PARSER_STD_REQUEST
} RX_PARSER_ACTION;

struct _cmd_parse
{
  unsigned short out_cmd;//out command
  bool ext;             //out command is ext?
  RX_PARSER_ACTION action;//proposed action
};

const struct _cmd_parse cmd_parse[] = {
                              {NetroMessage::INTERFACE_CONTROL_MODEM_CMD | (((unsigned short)NetroMessage::INTERFACE_MODEM_SET_MODE_CMD) << 8), false, RX_PARSER_STD_ANSWER},
                              {NetroMessage::INTERFACE_CONTROL_MODEM_CMD | (((unsigned short)NetroMessage::INTERFACE_MODEM_SAVE_DEV_ID_CMD) << 8), false, RX_PARSER_STD_ANSWER},
                              {NetroMessage::INTERFACE_CONTROL_MODEM_CMD | (((unsigned short)NetroMessage::INTERFACE_MODEM_DELETE_DEV_ID_CMD) << 8), false, RX_PARSER_STD_ANSWER},
                              {NetroMessage::INTERFACE_REQUEST_CMD | (((unsigned short)NetroMessage::INTERFACE_STD_PARAM_MODEMID) << 8), false, RX_PARSER_STD_REQUEST},
                              {NetroMessage::INTERFACE_REQUEST_CMD | (((unsigned short)NetroMessage::INTERFACE_STD_PARAM_VERSION) << 8), false, RX_PARSER_STD_REQUEST}
                            };

void SHModem::parseRXCommand()
{
  NetroMessage * tmpMessage = NetroMessage::createFromBuffer(_rcv_buf,_rcv_byte_num);
  if (!tmpMessage)
    return;
    
  //if message from sensor
  if (tmpMessage -> isExt() && tmpMessage -> command() == NetroMessage::INTERFACE_RESULT_BUVO_CMD)
  {
      //todo: parse sensor command
      delete tmpMessage;
      return;
  }
  
  //if we are not waitng for answer, then drop it
  if (isFree())
  {
    delete tmpMessage;
    return;
  }
    
  //parser
  RX_PARSER_ACTION parser =  RX_PARSER_STD_ANSWER;//default parser  
  for (int i = 0; i < sizeof(cmd_parse) / sizeof(struct _cmd_parse); i++)
    if (_msg -> isExt() == cmd_parse[i].ext && _msg -> command() == cmd_parse[i].out_cmd)
    {
      parser = cmd_parse[i].action;
      break;
    }
    
  switch (parser)
  {
  case RX_PARSER_STD_ANSWER:
    if ((tmpMessage -> command() == NetroMessage::INTERFACE_RESULT_STATUS_CMD && tmpMessage -> isExt()) || 
        (tmpMessage -> command() == (NetroMessage::INTERFACE_ANSWER_CMD | (((unsigned short)NetroMessage::INTERFACE_STD_PARAM_RESULT) << 8)) && !tmpMessage -> isExt()))
    {
      if (tmpMessage -> stdData() == NetroMessage::INTERFACE_RESULT_OK_CONST)
      {
        _free = true;
        _error = false;
#ifdef SH_TRANSPORT_DEBUG
        Serial.println("SH: correct standard answer");
#endif
        break;
      }
    }
#ifdef SH_TRANSPORT_DEBUG
    Serial.println("SH: Incorrect standard answer");
#endif
    break;
  case RX_PARSER_STD_REQUEST:
    if (!tmpMessage -> isExt() && (tmpMessage -> command() & 0xff00) == (_msg -> command() & 0xff00) && (tmpMessage -> command() & 0xff) == NetroMessage::INTERFACE_ANSWER_CMD)
    {
      _free = true;
      _error = false;
      if ((tmpMessage -> command() >> 8) == NetroMessage::INTERFACE_STD_PARAM_MODEMID)
      {//parsing caught sensor
        _sensor_id = tmpMessage -> stdData() | (((unsigned long)tmpMessage -> flags()) << 16);
        if (_sensor_id != (unsigned int)(-1))
          _listening = false;
      }
#ifdef SH_TRANSPORT_DEBUG
    Serial.println("SH: correct request answer");
#endif
      break;
    }
#ifdef SH_TRANSPORT_DEBUG
    Serial.println("SH: Incorrect request answer");
#endif
    break;
  default: 
    break;  
  }
  delete tmpMessage;
}


bool SHModem::setListenMode(bool value)
{
  if (!_free)
    return false;
  if (_listening == value)//no need to send any commands
    return true;
  NetroMessage * msg = NetroMessage::createStd(NetroMessage::INTERFACE_CONTROL_MODEM_CMD | (((unsigned short)NetroMessage::INTERFACE_MODEM_SET_MODE_CMD) << 8),0,value ? 2 : 1,0);
  sendCommand(*msg);
  delete msg;
  _listening = value;
  if (_listening)
    _listen_timeout = millis() + 120000;//2 minutes timeout
  return true;
}


void SHModem::getSensorId()
{
  NetroMessage * msg = NetroMessage::createStd(NetroMessage::INTERFACE_REQUEST_CMD | (((unsigned short)NetroMessage::INTERFACE_STD_PARAM_MODEMID) << 8),0,0,1);
  sendCommand(*msg);
  delete msg;
}

