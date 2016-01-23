#include <stdlib.h>
#include "NetroMessage.h"
#include "VoltageSensor.h"
#include "GSMModule.h"
#include "SHModem.h"

NetroMessage * msg;
VoltageSensor vs(0);
GSMModule gsm(Serial1);
SHModem sh(Serial3);

void setup() 
{
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  gsm.begin(115200);
  sh.begin(115200);
  msg = NetroMessage::createStd(0x1220,0,0,0);
  delay(10000); 
  //gsm.sendSMS("","hello");
}
unsigned long next_cmd_time = 0;
void loop() 
{ 
  //vs.proc();
  //gsm.proc(); 
  sh.proc();
  if (sh.isFree() && next_cmd_time < millis())
  {
    sh.sendCommand(*msg);
    next_cmd_time = millis()  + 5000;
  }
}

void setupSensor()
{

}

