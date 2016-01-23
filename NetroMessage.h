#ifndef _NETROMESSAGE_H_
#define _NETROMESSAGE_H_

#define INTERFACE_STAF_DATA    0x7d
#define INTERFACE_START_DATA   0x7e
#define INTERFACE_ADDRESS_DATA 0xFF
//тип команды
#define INTERFACE_CONTROL_DATA 0x03//управление    
//версия протокола- в зависимости от нее формируется и распарсивается команда по разному
#define INTERFACE_PROTOCOL_STD_DATA 0x01
#define INTERFACE_PROTOCOL_EXT_DATA 0x02
#define INTERFACE_SIZE_STD_DATA 0x08


//сдвиги внутри буфера команды USART для std команд
#define INTERFACE_FLAG_OFFSET   0
#define INTERFACE_SOF_SIZE 1
#define INTERFACE_ADDRESS_OFFSET (INTERFACE_FLAG_OFFSET + INTERFACE_SOF_SIZE)     
#define INTERFACE_CONTROL_OFFSET (INTERFACE_ADDRESS_OFFSET +  1)  
#define INTERFACE_PROTOCOL_OFFSET (INTERFACE_CONTROL_OFFSET + 1)  
#define INTERFACE_SIZE_OFFSET   (INTERFACE_PROTOCOL_OFFSET + 1)
#define INTERFACE_NETRO_GROUP_OFFSET (INTERFACE_SIZE_OFFSET + 1)
#define INTERFACE_NETRO_CMD_OFFSET  (INTERFACE_NETRO_GROUP_OFFSET + 2)
#define INTERFACE_NETRO_DATA_OFFSET (INTERFACE_NETRO_CMD_OFFSET + 2)
#define INTERFACE_NETRO_ATTRIBUTTE_OFFSET (INTERFACE_NETRO_DATA_OFFSET + 2) 
#define INTERFACE_CRC_OFFSET  (INTERFACE_NETRO_ATTRIBUTTE_OFFSET + 2)
#define INTERFACE_STD_PACKET_LENGTH (INTERFACE_CRC_OFFSET + 2)//максимальная длина пакета USART(раскодированного)

#define INTERFACE_EXT_CMD_OFFSET  (INTERFACE_SIZE_OFFSET + 1)
#define INTERFACE_EXT_FLAGS_OFFSET  (INTERFACE_EXT_CMD_OFFSET + 2)
#define INTERFACE_EXT_DATA_OFFSET (INTERFACE_EXT_FLAGS_OFFSET + 2)
#define INTERFACE_EXT_PACKET_LENGTH 128

class NetroMessage 
{
public:
//коды расширенных команд Interface(общие категории)
  typedef enum {
    INTERFACE_SNIFFER_CMD = 1,   //команды сниффера          
    INTERFACE_FU_CMD = 2,           //команда с прошивкой
    INTERFACE_INTRO3_CMD = 6,
    INTERFACE_RESULT_CMD = 9,   //ответы на команды(расширенные)
    INTERFACE_DEBUG_CMD = 0xf
  } INTERFACE_EXT_CMD_CODE_T;
  
  typedef enum {
    INTERFACE_MODEM_SET_MODE_CMD = 1,
    INTERFACE_MODEM_SET_CURRENT_DEV_CELL_CMD = 2,
    INTERFACE_MODEM_DELETE_DEV_CELL_CMD = 3,
    INTERFACE_MODEM_SET_TRANSMIT_PROTO_CMD = 4,
    INTERFACE_MODEM_SET_CURRENT_DEV_ID_CMD = 5,
    INTERFACE_MODEM_DELETE_DEV_ID_CMD = 6,
    INTERFACE_MODEM_SAVE_DEV_ID_CMD = 7,
    INTERFACE_MODEM_SET_NETRO_TIMEOUT_CMD = 8
  } INTERFACE_MODEM_CONTROL_CMD_T;
  
  //типы команд
  typedef enum {
    INTERFACE_INIT_WRITE_GROUP_CMD = 1,
    INTERFACE_INIT_WRITE_SCRIPT_CMD = 2,
    INTERFACE_INIT_WRITE_ALLOW_CMD = 3,
    INTERFACE_INIT_WRITE_STOP_CMD = 4,
  
    INTERFACE_WORK_GROUP_WRITE = 5,
    INTERFACE_WORK_ALLOW_WRITE = 6,
    INTERFACE_WORK_CHANGE_SCRIPT = 7,
    INTERFACE_WORK_DEL_GROUP = 8,
  
    INTERFACE_ACTION_CMD = 16,
    INTERFACE_SCANCOD_CMD = 17,
  
    INTERFACE_CONTROL_CMD = 24,
    INTERFACE_CONTROL_MODEM_CMD = 25,
    INTERFACE_REQUEST_CMD = 32,
    INTERFACE_ANSWER_CMD = 33,      
  } INTERFACE_STD_CMD_CODE_T;
  
  //команды ответов на расширенные команы
  typedef enum {
    INTERFACE_RESULT_STATUS_CMD = INTERFACE_RESULT_CMD | (((unsigned short)0) << 8),//результат выполнения команды
    INTERFACE_RESULT_BUVO_CMD = INTERFACE_RESULT_CMD | (((unsigned short)0x15) << 8),//команда снифера
    INTERFACE_RESULT_DEBUG_NETRO_CMD = INTERFACE_RESULT_CMD | (((unsigned short)0xf1) << 8)//команда статуса netro
  } INTERFACE_RESULT_CMD_T;

  //парметры для установки и запроса в стандартных командах
  typedef enum {
      INTERFACE_STD_PARAM_FREESPACE = 17,//подкоманда запроса свободного места в модеме
      INTERFACE_STD_PARAM_VERSION = 18,//подкоманда запроса версии ПО модема
      INTERFACE_STD_PARAM_LASTSAVED = 19, //запрос номера в таблице сохраненного устройства в ежиме инициализации модема
      INTERFACE_STD_PARAM_BTNSETUP = 20,
      INTERFACE_STD_PARAM_MODEMFUNCS = 21, //запрос функций модема и поддерживаемых модемом протоколов
      INTERFACE_STD_PARAM_MODEMID = 22 //id модема 
  } INTERFACE_STD_PARAM_T;
  
  // результат полученной команды USART, который передается в ответе
  typedef enum {
      INTERFACE_RESULT_OK_CONST   = 1,
      INTERFACE_RESULT_CRCERROR_CONST = 2,
      INTERFACE_RESULT_OVERFLOW_CONST = 3,
      INTERFACE_RESULT_WAIT_CONST = 4,
      INTERFACE_RESULT_DONTWAIT_CONST = 5,
      INTERFACE_RESULT_APPLY_OK_CONST   = 6,
      INTERFACE_RESULT_CONERROR_CONST   = 7,
      INTERFACE_RESULT_DEV_ERR_1_CONST = 10
  } INTERFACE_RESULT_T;
  
  typedef enum {
      INTERFACE_EVENT_NONE = 0,
      INTERFACE_EVENT_BYTE_IN,//словлен байт
      INTERFACE_EVENT_BAD_BYTE,//принят неверный байт(приемник работает)
      INTERFACE_EVENT_CMD_IN,//команда принята(приемник остановлен)
  }INTERFACE_EVENT_T;

  NetroMessage(const NetroMessage & msg);
	virtual ~NetroMessage();
	static NetroMessage * createStd(unsigned short cmd, unsigned short group, unsigned short data, unsigned short flags);
	static NetroMessage * createExt(unsigned short cmd, unsigned short flags, unsigned char * pBuf, unsigned char bufSize);
  NetroMessage & operator=(const NetroMessage & msg);

  //get command buffer
	unsigned char * buffer() const;

 //get command buffer size
	unsigned char size() const;

 //is command extendeded
  //bool isExt()const;

  //get command value
  //unsigned short command() const;

  
    
private:
#pragma pack(push, 1)
  typedef struct _std_cmd_header_t {
    unsigned char startFlag;
    unsigned char address;
    unsigned char operation;
    unsigned char protocol;
    unsigned char size;
  }STD_CMD_HEADER_T;
  
  typedef struct _std_cmd_t {
    STD_CMD_HEADER_T head;
    unsigned short group;
    unsigned short command;
    unsigned short data;
    unsigned short flags;
    unsigned short crc;
  } STD_CMD_T;

//short ext command(w/out data)
  typedef struct _ext_cmd_t {
    STD_CMD_HEADER_T head;
    unsigned short command;
    unsigned short flags;
    union {
      unsigned short crc;
      unsigned char data[1];
    } format;
  } EXT_CMD_T;
#pragma pack(pop)
	NetroMessage();
  static unsigned short calcCrc(unsigned char * data, unsigned char len);
	unsigned char * _buf;
  unsigned char _size;
};
#endif
