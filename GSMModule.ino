#include "GSMModule.h"

GSMModule::GSMModule(HardwareSerial &serial) :
  _serial(&serial),
  _first_time(1000),
  _intra_time(50),
  _overflow_size(0),
  _overflow_slot(0),
  _buf_size(0),
  _r_flag(false),
  _cb() {}


void GSMModule::setPhone(unsigned char element, const String & phone_number)
{
  if (element >= PHONE_NUMBER_COUNT)
    return;
  _phone_numbers[element] = phone_number;
  Serial.write("GSM: register phone ");
  Serial.write(phone_number.c_str());
  Serial.write("\n\r");
}


void GSMModule::begin(unsigned long baud) 
{
  _serial->begin(baud);
}

void GSMModule::end() {
  _serial->end();
}

void GSMModule::send(const char *cmd) 
{
  _serial->write(cmd);
  _serial->write('\r');

#ifdef ECHO_ENABLED
  console.print(F(":"));
  console.println(cmd);
#endif
}

void GSMModule::send_P(const char *cmd) 
{
  // Cleanup serial buffer
  while (_serial->available())
    recv();
  Serial.println((__FlashStringHelper *)cmd);
  //Serial.write('\r');
  _serial->print((__FlashStringHelper *)cmd);
  _serial->write('\r');

#ifdef ECHO_ENABLED
  console.print(F(":"));
  console.println((__FlashStringHelper *)cmd);
#endif
}

void GSMModule::handleCallback() {
  // Handle overflow request first
  size_t pos = 0;
  int i = _overflow_slot;
  if (_overflow_size > 0 && _cb[i].func)
    pos += _cb[i].func(_buf, _buf_size, _cb[i].data);

  // Handle the rest if we still have available space
  while (pos < _buf_size) {
    size_t used = 0;
    for (i = 0; i < GSM_MAX_CALLBACK; i++) {
      if (_cb[i].func && !strncmp_P((char *)_buf + pos, _cb[i].match, _cb[i].length)) {
        used = _cb[i].func(_buf + pos, _buf_size - pos, _cb[i].data);
        if (used)
          break;
      }
    }
    pos += used ? used : 1;
  }

  // Callback can request overflow by returning used space
  // beyond _buf space
  if (pos > _buf_size) {
    _overflow_size = pos - _buf_size;
    _overflow_slot = i;
  } else {
    _overflow_size = 0;
  }
}

size_t GSMModule::recv() {
  unsigned long timeout = millis() + _first_time;
  _buf_size = 0;
  while (millis() < timeout) {
    if (_serial->available()) {
      
      _buf[_buf_size++] = _serial->read();
      Serial.write(_buf[_buf_size-1]);
      if (_intra_time > 0)
        timeout = millis() + _intra_time;
      if (_buf_size >= GSM_BUFFER_SIZE)
        break;
    }
  }
  _buf[_buf_size] = 0;
#ifdef ECHO_ENABLED
  if (_buf_size > 0)
    console.write((byte *) _buf, _buf_size);
#endif
  handleCallback();
  return _buf_size;
}

char *GSMModule::find_P(const char *needle) {
  return strstr_P((char *)_buf, needle);
}

int GSMModule::recvUntil_P(const char *s1, const char *s2, const char *s3) {
  const char *ss[3] = { s1, s2, s3 };
  recv();
  for (int i = 0; i < 3; i++)
    if (ss[i] && strstr_P((char *)_buf, ss[i]) != NULL)
      return i + 1;
  return 0;
}

int GSMModule::recvUntil_P(int tries, const char *s1, const char *s2, const char *s3) {
  int ret = 0;
  for (int i = 0; i < tries && ret == 0; i++)
    ret = recvUntil_P(s1, s2, s3);
  return ret;
}

void GSMModule::setTimeout(long first_time, long intra_time) {
  _first_time = first_time;
  _intra_time = intra_time;
}

void GSMModule::addTask(const GSMTask & task)
{
  GSMTask _task(task);
  send(_task.gsmString().c_str());
}

void GSMModule::proc() {
  //if (_serial->available())
  //  recv();
  while(_serial -> available())
  {
    _buf[_buf_size] = _serial -> read();
    //ignoring start \r and \n symbols
    if (!_buf_size && (_buf[_buf_size] == '\n' || _buf[_buf_size] == '\r'))
      continue;
    Serial.write(_buf[_buf_size]);
    if (_buf[_buf_size] == '\n' && _r_flag)
    {
      //parse string
      parse(_buf, _buf_size + 1);
      _buf_size = 0;
      _r_flag = false;
      continue;
    }
    else
      if (_buf[_buf_size] == '\r')
        _r_flag = true;
    _buf_size++;
  }

}

typedef enum {
  GSM_PARSER_INCOME_SMS,
  GSM_PARSER_INCOME_RING,
  GSM_PARSER_GET_SMS,
  GSM_PARSER_DELETE_SMS  
} GSM_PARSER;

typedef struct _cmd_parser {
  String cmd;
  GSM_PARSER parser;
} CMD_PARSER;
#define PARSE_CMDS 2
const CMD_PARSER parser[PARSE_CMDS] = {
                              {"+CMTI: \"SM\",",GSM_PARSER_INCOME_SMS},
                              {"RING",GSM_PARSER_INCOME_RING}
};
void GSMModule::parse(byte * _buf, size_t size)
{
  Serial.write("GSM IN:");
  for (size_t i = 0; i < size; i++)
  {
    Serial.write(_buf[i]);
    //Serial.print(_buf[i],HEX);
    // Serial.print(" ");
  }
  //Serial.write("\n\rsize:");
  //Serial.println(size,DEC);
  
  int parse_index;
  for (parse_index = 0; parse_index < PARSE_CMDS; parse_index++)
  {
    //Serial.write("compare length ");
    //Serial.println(parser[i].cmd.length(),DEC);
      
    int res = memcmp(parser[parse_index].cmd.c_str(),_buf,parser[parse_index].cmd.length());
    /*if(res != 0)
      Serial.println(res,DEC);
    else
      Serial.write("parse ok\r\n");*/
    if (!res)
      break;
  }

  if (parse_index == PARSE_CMDS)
    return;
            
  switch(parser[parse_index].parser)
  {
  case GSM_PARSER_INCOME_SMS:
  {
    String sms_in = String((char *)_buf + parser[parse_index].cmd.length());
    long sms_num = sms_in.toInt();
    //Serial.println(sms_num,DEC);
    if (sms_num)
    {
      GSMTask::GSM_READ_SMS_T task_data;
      task_data.number = sms_num;
      addTask(GSMTask(GSMTask::GSM_TASK_READ_SMS,&task_data));
    }
    break;
  }
  case GSM_PARSER_INCOME_RING:
    break;
  default:
    return;  
  }
}

void GSMModule::setCallback_P(int slot, const char *match, callback_func func, void *data) {
  _cb[slot].match = match;
  _cb[slot].length = strlen_P((char *)match);
  _cb[slot].data = data;
  _cb[slot].func = func;
}


boolean GSMModule::isModemReady() {
  boolean ready = false;
  for (int i = 0; i < 2 && !ready; i++)
    ready = sendRecvUntil_P(G("AT"), G("OK"));
  if (ready)
    sendRecv_P(G("ATE0"));
  return ready;
}

boolean GSMModule::isRegistered() {
  return sendRecvUntil_P(G("AT+CREG?"), G("+CREG: 0,1"));
}

boolean GSMModule::isAttached() {
  return sendRecvUntil_P(G("AT+CGATT?"), G("+CGATT: 1"));
}

#define IMEI_LENGTH 14

boolean GSMModule::getIMEI(char *imei) {
  if (!sendRecvUntil_P(G("AT+GSN"), G("OK")))
    return false;
  int len = 0;
  for (size_t i = 0; i < _buf_size && len < IMEI_LENGTH; i++) {
    if ('0' <= _buf[i] && _buf[i] <= '9')
      imei[len++] = _buf[i];
    else if (len > 0)
      break;
  }
  imei[len] = 0;
  return true;
}


bool GSMModule::sendSMS(const String & number, const String & text)
{
  String sms_cmd;
  bool ok = false;
  //switch to text sms mode
  for (int i = 0; i < 2 && !ok; i++)
    ok = sendRecvUntil_P(G("AT+CMGF=1"), G("OK"));
  if (!ok)
    return false;
  //set sms encoding
  ok = false;
  for (int i = 0; i < 2 && !ok; i++)
    ok = sendRecvUntil_P(G("AT+CSCS= \"GSM\""), G("OK"));
  if (!ok)
    return false;
  //fill phone number 
  ok = false;
  sms_cmd = "AT+CMGS=\"" + number + "\"\r\n";
  for (int i = 0; i < 2 && !ok; i++)
    ok = sendRecvUntil_P(sms_cmd.c_str(), G(">"));    
  if (!ok)
    return false;
  //set sms text 
  ok = false;
  setTimeout(5000,0);
  sms_cmd = text + "\x1a";
  ok = sendRecvUntil_P(sms_cmd.c_str(), G("OK"));  
  setTimeout(1000,50); 
  //todo: if not ok, do escape from message mode 
  return ok;
}

