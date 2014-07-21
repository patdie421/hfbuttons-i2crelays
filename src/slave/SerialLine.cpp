#include <Arduino.h>
#include "SerialLine.h"


SerialLine::SerialLine(HardwareSerial *s)
{
  serial=s;
  flush();
}


#ifdef USBCON
SerialLine::SerialLine(Serial_ *s)
{
  serial=(HardwareSerial *)s;
  flush();
}
#endif


void SerialLine::flush()
{
  lineIndexIn=0;
  lineIndexOut=0;
  lineNbCars=0;
}


int SerialLine::available()
{
  return lineNbCars;
}


int SerialLine::readByte()
{
  if(lineNbCars)
  {
    char c = line[lineIndexOut];
    lineNbCars--;
    lineIndexOut++;
    if(lineIndexOut==LINE_MAX_CARS)
      lineIndexOut=0;
    return c;
  }
  else
    return -1;
}


int SerialLine::parseInt()
{
  if(lineNbCars)
  {
    char strInt[MYPARSEINTBUFFSIZE];
    int i = 0;
    do
    {
      if(line[lineIndexOut] >= '0' && line[lineIndexOut] <='9')
        strInt[i++]=readByte();
      else
        break;
    }
    while(i<(MYPARSEINTBUFFSIZE-1) && lineNbCars);
    if(i>0)
    {
      strInt[i]=0;
      return atoi(strInt);
    }
    else
      return -1;
  }
  else
    return -1;
}


int SerialLine::readLine()
{
  if(!serial)
    return -1;
    
  if(lineNbCars < LINE_MAX_CARS)
  {
    while(serial->available())
    {
      char c=serial->read();
      if(c==13)
      {
        return lineNbCars;
      }
      if( (c>='0' && c<='9')
        || (c>='A' && c<='Z')
        || (c>='a' && c<='z')
        || c == ':'
        || c == ';'
        || c == '$'
        || c == '#' )
      {
        serial->print(c);
        line[lineIndexIn]=c;
        lineNbCars++;
        lineIndexIn++;
        if(lineIndexIn==LINE_MAX_CARS)
          lineIndexIn=0;
      }
      return 0;
    }
  }
  else
    return -1;
  return 0;
}


int SerialLine::writeByte(byte c)
{
  if(serial)
    return serial->print(c);
  else
    return -1;
}


int SerialLine::writeLine(char * l)
{
  if(serial)
    return serial->print(l);
  else
    return -1;
}


int SerialLine::writeHex(byte c)
{
  if(serial)
  {
    if (c<16)
      serial->print("0");
    return serial->print(c, HEX);
  }
  else
    return -1;
}

