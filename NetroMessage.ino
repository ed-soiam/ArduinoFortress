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


NetroMessage * NetroMessage::createFromBuffer(unsigned char * pBuf, unsigned char bufSize)
{
  unsigned short crc,tmpCrc;
  //parsing buffer, check crc, header, minimal lengths and so on
  if (bufSize < INTERFACE_SIZE_OFFSET + 1 + 2)//head+crc
    return 0;
  //check total length
  if (bufSize != INTERFACE_SIZE_OFFSET + 1 + ((STD_CMD_T *)pBuf) -> head.size + 2)
    return 0;
  //check correct crc
  tmpCrc = calcCrc((unsigned char *)&(((STD_CMD_T *)pBuf) -> head.address), bufSize - 3);
  memcpy(&crc,pBuf + bufSize - sizeof(crc),sizeof(crc));
  if (crc != tmpCrc)
    return 0;
    
  switch (((STD_CMD_T *)pBuf) -> head.protocol)
  {
  case INTERFACE_PROTOCOL_STD_DATA:
    if (memcmp(pBuf,std_cmd_head,sizeof(std_cmd_head)))
      return 0;
    break;
  case INTERFACE_PROTOCOL_EXT_DATA:
    if (memcmp(pBuf,ext_cmd_head,sizeof(ext_cmd_head)))
        return 0;
    break;
  default:
    return 0;
  }

  NetroMessage * msg = new NetroMessage();
  msg -> _size = bufSize;
  msg -> _buf = new unsigned char[msg -> _size];
  memcpy(msg -> _buf, pBuf, msg -> _size);
  return msg;
}


unsigned char * NetroMessage::buffer() const
{
  return _buf;
}


unsigned char NetroMessage::size() const
{
  return _size;
}


bool NetroMessage::isExt()const
{
  return ((STD_CMD_T *)_buf) -> head.protocol == INTERFACE_PROTOCOL_STD_DATA ? false : true;
}


unsigned short NetroMessage::command() const
{
  switch (((STD_CMD_T *)_buf) -> head.protocol)
  {
  case INTERFACE_PROTOCOL_STD_DATA:
    return ((STD_CMD_T *)_buf) -> command;
  case INTERFACE_PROTOCOL_EXT_DATA:
    return ((EXT_CMD_T *)_buf) -> command;
  default:
    return 0;
  }
}


unsigned short NetroMessage::flags() const
{
  switch (((STD_CMD_T *)_buf) -> head.protocol)
  {
  case INTERFACE_PROTOCOL_STD_DATA:
    return ((STD_CMD_T *)_buf) -> flags;
  case INTERFACE_PROTOCOL_EXT_DATA:
    return ((EXT_CMD_T *)_buf) -> flags;
  default:
    return 0;
  }
}


unsigned short NetroMessage::group() const
{
  switch (((STD_CMD_T *)_buf) -> head.protocol)
  {
  case INTERFACE_PROTOCOL_STD_DATA:
    return ((STD_CMD_T *)_buf) -> group;
  default:
    return 0;
  }
}


unsigned short NetroMessage::stdData() const
{
  switch (((STD_CMD_T *)_buf) -> head.protocol)
  {
  case INTERFACE_PROTOCOL_STD_DATA:
    return ((STD_CMD_T *)_buf) -> data;
  case INTERFACE_PROTOCOL_EXT_DATA:
  {
    unsigned short res = 0;
    if (((EXT_CMD_T *)_buf) -> head.size >= 6)
      memcpy(&res,((EXT_CMD_T *)_buf) -> format.data,sizeof(res));
    return res;
  }    
  default:
    return 0;
  }
}


void NetroMessage::extData(unsigned char * pBuf, unsigned char * bufSize) const
{
  switch (((STD_CMD_T *)_buf) -> head.protocol)
  {
  case INTERFACE_PROTOCOL_STD_DATA:
    *bufSize = min(*bufSize,sizeof(((STD_CMD_T *)_buf) -> data));
    if (*bufSize)
      memcpy(pBuf,&((STD_CMD_T *)_buf) -> data, *bufSize);
    return;
  case INTERFACE_PROTOCOL_EXT_DATA:
    *bufSize = min(*bufSize,((STD_CMD_T *)_buf) -> head.size - 6);//2-crc,2-cmd,2-flags);
    if (*bufSize)
      memcpy(pBuf,((EXT_CMD_T *)_buf) -> format.data, *bufSize);
    return;
  default:
    return;
  }
}

  
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
