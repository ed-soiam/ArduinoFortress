#ifndef _EEPROM_MANAGER_H_
#define _EEPROM_MANAGER_H_
#include "defs.h"
class EEPROMManager {
public:
  typedef enum {
    EEPROM_PHONE_PART,
    EEPROM_SENSOR_PART
  } EEPROM_PART_T;
#pragma pack(push, 1)
  typedef struct _phone_element
  {
    unsigned char flags;
    char number[PHONE_NUMBER_LENGTH];
  } PHONE_ELEMENT_T;

  typedef struct _sensor_element
  {
    unsigned char type;
    char name[SENSOR_NAME_LENGTH];
    union {
      unsigned char info[8];
      struct {
        unsigned char id;
        unsigned char pin;
        unsigned char lowOn;
        unsigned short compare;
      } analog_sensor;      
      unsigned long id;
    };
  } SENSOR_ELEMENT_T;
  
  EEPROMManager();

  static bool load(EEPROM_PART_T part, unsigned char element,unsigned char * pBuf);
  static bool save(EEPROM_PART_T part, unsigned char element,unsigned char * pBuf);
private:

  typedef struct _inner_eeprom
  {
    PHONE_ELEMENT_T phone[PHONE_NUMBER_COUNT];
    SENSOR_ELEMENT_T sensor[SENSOR_COUNT];
  } INNER_EEPROM;
};
#pragma pack(pop)
#endif
