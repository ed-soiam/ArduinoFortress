#include <string.h>
#include "NetroMessage.h"
const unsigned char std_cmd_head[5] = {INTERFACE_START_DATA,INTERFACE_ADDRESS_DATA,INTERFACE_CONTROL_DATA,INTERFACE_PROTOCOL_STD_DATA,INTERFACE_SIZE_STD_DATA};
const unsigned char ext_cmd_head[4] = {INTERFACE_START_DATA,INTERFACE_ADDRESS_DATA,INTERFACE_CONTROL_DATA,INTERFACE_PROTOCOL_EXT_DATA};
NetroMessage::NetroMessage()
{
  _buf = 0;
  _size = 0;
}

NetroMessage::NetroMessage(const NetroMessage & msg)
{
  this -> _size = msg._size;
  if (msg._buf)
  {
    this -> _buf = new unsigned char[this -> _size];
    memcpy(this -> _buf, msg._buf, this -> _size);
  }
}


NetroMessage & NetroMessage::operator=(const NetroMessage & msg)
{
  *this = NetroMessage(msg);
  return *this;
}

NetroMessage::~NetroMessage()
{
  delete [] _buf;
}

NetroMessage * NetroMessage::createStd(unsigned short cmd, unsigned short group, unsigned short data, unsigned short flags)
{
  unsigned short crc;
  NetroMessage * msg = new NetroMessage();
  msg -> _size = sizeof(STD_CMD_T);
  msg -> _buf = new unsigned char[msg -> _size];
  memset(msg -> _buf,0,msg -> _size);
  //filling command
  memcpy(msg -> _buf,std_cmd_head,sizeof(std_cmd_head));
  ((STD_CMD_T *)msg -> _buf) -> group = group; 
  ((STD_CMD_T *)msg -> _buf) -> command = cmd;
  ((STD_CMD_T *)msg -> _buf) -> flags = flags;
  ((STD_CMD_T *)msg -> _buf) -> data = data; 
  crc = calcCrc((unsigned char *)&(((STD_CMD_T *)msg -> _buf) -> head.address), msg -> _size - 3);
  ((STD_CMD_T *)msg -> _buf) -> crc = crc;
  return  msg;
}

NetroMessage * NetroMessage::createExt(unsigned short cmd, unsigned short flags, unsigned char * pBuf, unsigned char bufSize)
{
  unsigned short crc;
  NetroMessage * msg = new NetroMessage();
  msg -> _size = sizeof(EXT_CMD_T) + bufSize;
  msg -> _buf = new unsigned char[msg -> _size];
  memset(msg -> _buf,0,msg -> _size);
  //filling command
  memcpy(msg -> _buf,ext_cmd_head,sizeof(ext_cmd_head));
  ((EXT_CMD_T *)msg -> _buf) -> command = cmd;
  ((EXT_CMD_T *)msg -> _buf) -> flags = flags;
  ((EXT_CMD_T *)msg -> _buf) -> head.size = 4 + bufSize;
  memcpy(((EXT_CMD_T *)msg -> _buf) -> format.data,pBuf,bufSize);
  crc = calcCrc((unsigned char *)&(((STD_CMD_T *)msg -> _buf) -> head.address), msg -> _size - 3);
  memcpy(&msg -> _buf[msg -> _size - 2],&crc,sizeof(crc));
  return  msg;
}

unsigned char * NetroMessage::buffer() const
{
  return _buf;
}

unsigned char NetroMessage::size() const
{
  return _size;
}


//подсчет crc массива data длиной len
unsigned short NetroMessage::calcCrc(unsigned char * data, unsigned char len)
{
  unsigned char i;
  unsigned short crc = 0x0000;
  unsigned char crc_data;
  //не знаю, как это должно работать, не мое
  while (len--)
  {
          crc_data = *data++;
  
          for (i = 0; i < 8; i++)
          {
                  if ( crc & 0x8000 )
                  {
                          crc <<= 1;
                          if (crc_data & 0x0001)
                                  crc |= 0x0001;
                          crc ^= 0x1021;
                  }
                  else
                  {
                          crc <<= 1;
                          if (crc_data & 0x0001)
                                  crc |= 0x0001;
                  }
                  crc_data >>= 1;
          }

  }  
  return crc;
}
