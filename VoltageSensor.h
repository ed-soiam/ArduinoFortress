#ifndef _VOLTAGESENSOR_H_
#define _VOLTAGESENSOR_H_
class VoltageSensor
{
public:
  VoltageSensor(unsigned char pin);
  void proc();
private:
  unsigned char pin; 
};
#endif