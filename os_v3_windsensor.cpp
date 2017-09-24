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

#include "Arduino.h"
#include "os_v3_windsensor.h"

OS_v3_windsensor::OS_v3_windsensor(int transmitterPin)
{
  m_transmitterPin = transmitterPin;

  // new rolling code with every reset
  randomSeed(analogRead(1)); // analog pin one used as source of noise for random seed
  m_rollingCode = random(0x01, 0xFE);

  pinMode(m_transmitterPin, OUTPUT);
}


void OS_v3_windsensor::setChannel(uint8_t channel)
{
  m_packet[5] |= channel;
}

void OS_v3_windsensor::setAvgWind(unsigned int avgWind)
{

  //m_packet[10] = ((avgWind) % 10) << 4;
  m_packet[10] |= (avgWind) % 10;
  m_packet[11]  = ((avgWind / 10) % 10) << 4;
  m_packet[11] |= (avgWind / 100) % 10;

}

void OS_v3_windsensor::setGust(unsigned int gust)
{
  m_packet[9]  = (gust % 10) << 4;
  m_packet[9] |= ((gust / 10) % 10);
  m_packet[10] |= ((gust / 100) % 10) << 4;; // & 0xF0;

}

void OS_v3_windsensor::setDirection(unsigned long m_direction)
{
  delay(10);
}

void OS_v3_windsensor::setBatteryStatus(bool batterystatus)
{
  if (!batterystatus)
    bitSet(m_packet[7], 6);

  else
    bitClear(m_packet[7], 6);
}

unsigned char OS_v3_windsensor::calcChecksum(void)
{
  unsigned char checksum = 0;

  checksum += m_packet[3] & 0x0F;

  for (int i = 4; i < 12; i++)
  {
    checksum += (m_packet[i] >> 4) & 0x0F;
    checksum += (m_packet[i] & 0x0F);
  }

  //checksum += (m_packet[12] >> 4) & 0x0F;
  // nibble swap
  return ((checksum & 0x0F) << 4) | ((checksum & 0xF0) >> 4);

  return checksum;

}

void OS_v3_windsensor::buildAndSendPacket()
{
  // Nibbles are sent LSB first

  // --- preamble ---
  // The preamble consists of twenty four '1' bits (6 nibbles) for v3.0 sensors
  m_packet[0] = 0xFF;
  m_packet[1] = 0xFF;
  m_packet[2] = 0xFF;

  // A sync nibble (4-bits) which is '0101'
  m_packet[3] = 0xA0;

  // nibbles 0..3 Sensor ID This 16-bit value is unique to each sensor, or sometimes a group of sensors.
  m_packet[3] |= 0x01;
  m_packet[4] = 0x98;
  m_packet[5] = 0x40;

  // nibbles 5..6 Rolling Code Value changes randomly every time the sensor is reset
  m_packet[6] = m_rollingCode;

  unsigned char checksum = calcChecksum();

  // Checksum
  m_packet[12] = checksum;

  // send the data over rf
  sendData();
}

void OS_v3_windsensor::sendData(void)
{
  int i;

  digitalWrite(m_transmitterPin, LOW);
  delayMicroseconds(2000);

  for (i = 0; i < sizeof(m_packet); i++)
  { //original = (i + 1 == sizeof(m_packet)));
    manchesterEncode(m_packet[i], (i + 1 == sizeof(m_packet)));
  }

  digitalWrite(m_transmitterPin, LOW);
}


void OS_v3_windsensor::manchesterEncode (unsigned char encodeByte, bool lastByte)
{
  unsigned char mask = 0x10;
  int loopCount;
  static int lastbit = 0;
  static unsigned long baseMicros = micros();

  //  488us timing would be 1024 data rate
  // the data rate actually appears to be 1020 or 490us
  // if the timing is off, then the base station won't receive packets reliably
  const unsigned int desiredDelay = 490;

  // due to processing time, the delay shouldn't be a full desired delay time
  const unsigned int shorten = 32;

  // bits are transmitted in the order 4 thru 7, then 0 thru 3

  for (loopCount = 0; loopCount < 8; loopCount++)
  {
    baseMicros += desiredDelay;
    unsigned long delayMicros = baseMicros - micros();

    if (delayMicros > 2 * delayMicros)
    {
      // big delay indicates break between packet transmits, reset timing base
      baseMicros = micros();
    }
    else if (delayMicros > 0)
    {
      delayMicroseconds(delayMicros);
    }

    if ((encodeByte & mask) == 0)
    {
      // a zero bit is represented by an off-to-on transition in the RF signal
      digitalWrite(m_transmitterPin, LOW);
      delayMicroseconds(desiredDelay - shorten);
      digitalWrite(m_transmitterPin, HIGH);

      // no long delay after last low to high transistion as no more data follows
      if (lastByte) delayMicroseconds(desiredDelay);

      lastbit = 0;
    }
    else
    {
      digitalWrite(m_transmitterPin, HIGH);
      delayMicroseconds(desiredDelay - shorten);
      digitalWrite(m_transmitterPin, LOW);
      lastbit = 1;
    }

    if (mask == 0x80)
    {
      mask = 0x01;
    }
    else
    {
      mask <<= 1;
    }

    baseMicros += desiredDelay;
  }
}



