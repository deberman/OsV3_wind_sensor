//Jack Christensen 19 Nov 2013
//CC BY-SA, see http://creativecommons.org/licenses/by-sa/3.0/
//
// I Found it here: https://github.com/epkboan/rain_sensor
//
// simplified and  made into an Arduino library
// and  modified to simulate a windsensor by deberman 2016-02-24
// 
// Library for simulating a WGR800 sensor using Oregon Scientific protocol V3
// the packet is built with the required WGR800 fields checksum  is calculated
// the data is manchester encoded and sent to the transmitter



#include "os_v3_windsensor.h"

OS_v3_windsensor windsensor(7);// pin 7 to transmitter data pin

//OS_v3_windsensor windsensor(5);// pin 7 to transmitter data pin

void setup(void)
{
  Serial.begin(115200);
  delay(300);
  
  windsensor.setChannel(5);// 1 to 15 is valid
  
}

void loop(void)
{
  windsensor.setAvgWind(999);// 1 = 0.1mm
  
  windsensor.setGust(999);// 1 = 0.1mm 

  windsensor.setDirection(0);// 1 = 0.1mm 
  
  windsensor.setBatteryStatus(false);// battery OK
  
  windsensor.buildAndSendPacket();
  
  Serial.println("packet sent");
  //while(1);
  delay(10000);
}
