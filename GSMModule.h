#ifndef _GSMMODULE_H_
#define _GSMMODULE_H_
#include "defs.h"
#include "GSMTask.h"
#define GSM_BUFFER_SIZE 64
#define GSM_MAX_CALLBACK 3
class GSMModule {
public:
	GSMModule(HardwareSerial &serial);

  void setPhone(unsigned char element, const String & phone_number);
	void begin(unsigned long baud);
	void end();

  void addTask(const GSMTask & task);

	// Set timeout for recv() and recvUntil()
	void setTimeout(long first_time = 1000, long intra_time = 50);

	// Must be called frequently to check incoming data
	void proc();

	inline HardwareSerial &serial() { return *_serial; }

private:
  // Send AT command.  Don't use '\r' as command suffix.
  void send(const char *cmd);
  //parse standard income initiative string from modem
  bool parseSTD(byte * _buf, size_t size);
  
  
	HardwareSerial *_serial;
  String _phone_numbers[PHONE_NUMBER_COUNT];
	byte _buf[GSM_BUFFER_SIZE];
	byte _buf_eol; // dummy EOL for string safety
	size_t _buf_size;
	unsigned long _first_time;
	unsigned long _intra_time;
	size_t _overflow_size;
	byte _overflow_slot;
  bool _r_flag;//'\r' was caught in last byte
  GSMTask task;
  unsigned long _rcv_timeout;
};

#endif
