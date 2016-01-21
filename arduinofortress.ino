#include <stdlib.h>
#include "NetroMessage.h"
#include "VoltageSensor.h"
#include "GSMModule.h"
#include "SHModem.h"

void led_dalay_proc();
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
  delay(20000); 
  //gsm.sendSMS("","hello");
}
bool ok = false;
void loop() 
{ 
  led_dalay_proc();                
  //vs.proc();
  //gsm.proc(); 
  sh.proc();
  sh.sendCommand(*msg,SHModem::WAIT_ANSWER_PARAM,0);
  /*if (gsm.isRegistered())
  {
      
  }*/
}


void led_dalay_proc()
{
  // visible watchdog :)
  digitalWrite(13, HIGH);       
  delay(1000);                  
  digitalWrite(13, LOW);        
  delay(1000);
}

