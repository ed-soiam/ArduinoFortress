#ifndef _GSMMODULE_H_
#define _GSMMODULE_H_

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Print.h>
#include <Client.h>
#include <IPAddress.h>
#include <avr/pgmspace.h>
#include "defs.h"
#include "GSMTask.h"
#define GSM_BUFFER_SIZE 64
#define GSM_MAX_CALLBACK 3

/*// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif*/

// Arduino F() macro will create PROGMEM string of type (const __FlashStringHelper *).
// This G() macro will cast it into (const char *) for use with C library.
#define G(s) (const char *)F(s)

// Functions with name ended with _P should be provided with
// PROGMEM value for its const char * parameter.
class GSMModule {
public:
	GSMModule(HardwareSerial &serial);

  void setPhone(unsigned char element, const String & phone_number);
	void begin(unsigned long baud);
	void end();

	// Send AT command.  Don't use '\r' as command suffix.
	void send(const char *cmd);
	void send_P(const char *cmd);
  void addTask(const GSMTask & task);
  //parse income string
  void parse(byte * _buf, size_t size);
	// Receive until buffer is full or timeout.
	size_t recv();

	// Find string inside receive buffer
	char *find_P(const char *needle);

	// Receive until specified token found or timeout.
	// Returns 1, 2, 3 depending on matched parameter. Or 0 if none found.
	int recvUntil_P(const char *s1, const char *s2 = NULL, const char *s3 = NULL);
	int recvUntil_P(int tries, const char *s1, const char *s2 = NULL, const char *s3 = NULL);

	// Utility functions
	inline size_t sendRecv_P(const char *cmd) {
		send_P(cmd);
		return recv();
	}
	inline int sendRecvUntil_P(const char *cmd,
			const char *s1, const char *s2 = NULL, const char *s3 = NULL) {
		send_P(cmd);
		return recvUntil_P(s1, s2, s3);
	}
	inline int sendRecvUntil_P(const char *cmd, int tries,
			const char *s1, const char *s2 = NULL, const char *s3 = NULL) {
		send_P(cmd);
		return recvUntil_P(tries, s1, s2, s3);
	}

	// Set timeout for recv() and recvUntil()
	void setTimeout(long first_time = 1000, long intra_time = 50);

	// Must be called frequently to check incoming data
	void proc();

	typedef size_t (*callback_func)(byte *buf, size_t length, void *data);
	void setCallback_P(int slot, const char *match, callback_func func, void *data);
	// Modem status testing functions
	boolean isModemReady();
	boolean isRegistered();
	boolean isAttached();
  bool sendSMS(const String & number, const String & text);
	boolean getIMEI(char *buf);

	inline HardwareSerial &serial() { return *_serial; }

private:
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

	struct {
		callback_func func;
		const char *match;
		byte length;
		void *data;
	} _cb[GSM_MAX_CALLBACK];

	void handleCallback();
  unsigned long _rcv_timeout;
};

#endif
