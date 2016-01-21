#include <stdlib.h>
#include "NetroMessage.h"
#include "VoltageSensor.h"
#include "GSMModule.h"
#include "SHModem.h"

void led_dalay_proc();
NetroMessage * msg;
VoltageSensor vs(0);
GSMModule gsm(Serial1);
SHModem sh(Serial2);

void setup() 
{
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  gsm.begin(115200);
  msg = NetroMessage::createStd(0,1,1,1);
}

void loop() 
{
  led_dalay_proc();                
  vs.proc();
  gsm.proc(); 
  sh.proc();
  if (gsm.isModemReady())
    Serial.print("gsm is attached\n\r");
}


void led_dalay_proc()
{
  // visible watchdog :)
  digitalWrite(13, HIGH);       
  delay(1000);                  
  digitalWrite(13, LOW);        
  delay(1000);
}

