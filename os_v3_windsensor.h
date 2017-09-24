//Jack Christensen 19 Nov 2013
//CC BY-SA, see http://creativecommons.org/licenses/by-sa/3.0/
//
//I Found it here: https://github.com/epkboan/rain_sensor
//
// simplified and  made into an Arduino library
// and  modified to simulate a windsensor by deberman 2016-02-24

// Library for simulating a WGR800 sensor using Oregon Scientific protocol V3
// the packet is built with the required WGR800 fields checksum  is calculated
// the data is manchester encoded and sent to the transmitter

#ifndef __OS_V3_WINDSENSOR_H_
#define __OS_V3_WINDSENSOR_H_

#include "Arduino.h"

class OS_v3_windsensor
{
  public:
  
  /* The transmitter, transmitterPin is the pin 
   * connected to the tx modules data pin
   * 
   * OS_v3_windsensor pcr800(1); Uses digital pin 1 to transmitt
   */
    OS_v3_windsensor(int transmitterPin);

    /* Set the channel for the transmitter
     *  1 to 15 is valid
     */
    void setChannel(uint8_t channel);

    /* The rain rate, in my implementation 
     *  the rate is set in 0.1 millimeters
     *  with precision of 0.01 inch 
     *  setRainRate(10) sends 1mm to the receiver
     *  TODO decide on better scheme to handle the
     *  conversion
     */
    void setAvgWind(unsigned int avgWind);

     /* The total rain accumulated, in my implementation 
     *  the value is set in 0.1 millimeters.
     *  
     *  setTotalRain(10) sends 1mm to the receiver
     */
    void setGust(unsigned int gust);

    /* The direction of the wind  
     *  
     *  setDirection(xx) sends xx to the receiver
     */
    void setDirection(unsigned long gust);

    /*Battery status
     * true = battery Ok
     * false = battery low
     */
    void setBatteryStatus(bool batterystatus);

    /*Call this when your parameters are set
     * and you are ready to send
     */
    void buildAndSendPacket();

  private:
  
    unsigned char m_packet[15];
    unsigned int m_rollingCode;
    uint8_t m_transmitterPin;

    unsigned char calcChecksum(void);
    void sendData(void);
    void manchesterEncode (unsigned char encodeByte, bool lastByte);
};

#endif

