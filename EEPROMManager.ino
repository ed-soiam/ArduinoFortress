#if defined (__AVR_ATmega128__)
#include <EEPROM.h>
#endif
#include "EEPROMManager.h"
EEPROMManager::EEPROMManager()
{
  
}


bool EEPROMManager::load(EEPROM_PART_T part, unsigned char element,unsigned char * pBuf)
{
#if defined (__AVR_ATmega128__)
  switch (part)
  {
  case EEPROM_PHONE_PART:
    if (element >= PHONE_NUMBER_COUNT)
      return false;
    for (unsigned char i = 0; i < sizeof(PHONE_ELEMENT_T); i++)
      *pBuf++ = EEPROM.read(offsetof(INNER_EEPROM,phone[element]) + i);
    break;
  case EEPROM_SENSOR_PART:
    if (element >= SENSOR_COUNT)
      return false;
    for (unsigned char i = 0; i < sizeof(SENSOR_ELEMENT_T); i++)
      *pBuf++ = EEPROM.read(offsetof(INNER_EEPROM,sensor[element]) + i);
    break;
  default:
    return false;
  }
  return true;
#else
  return false;
#endif
}


bool EEPROMManager::save(EEPROM_PART_T part, unsigned char element,unsigned char  * pBuf)
{
#if defined (__AVR_ATmega128__)
  switch (part)
  {
  case EEPROM_PHONE_PART:
    for (unsigned char i = 0; i < sizeof(PHONE_ELEMENT_T); i++)
    {
       EEPROM.write(offsetof(INNER_EEPROM,phone[element]) + i,*pBuf++);
      //Serial.print(offsetof(INNER_EEPROM,phone[element]) + i,DEC);
      //Serial.print(" ");
    }
   //Serial.print("\n\r");
    break;
  case EEPROM_SENSOR_PART:
    for (unsigned char i = 0; i < sizeof(SENSOR_ELEMENT_T); i++)
      EEPROM.write(offsetof(INNER_EEPROM,sensor[element]) + i,*pBuf++);
    break;
  default:
    return false;
  }
  return true;
#else
  return false;
#endif
}


