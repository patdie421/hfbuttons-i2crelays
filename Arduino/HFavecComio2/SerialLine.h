#ifndef __SERIALLINE__
#define __SERIALLINE__
#include <arduino.h>
#define MYPARSEINTBUFFSIZE 10
#define LINE_MAX_CARS 40

class SerialLine
{
private:
  char line[LINE_MAX_CARS];
  int lineIndexIn;
  int lineIndexOut;
  int lineNbCars;
  HardwareSerial *serial;
  
public:
  SerialLine(HardwareSerial *s);
#ifdef USBCON
  SerialLine(Serial_ *s);
#endif
  int readByte();
  int parseInt();
  int readLine();
  int available();
  void flush();
  int writeByte(byte c);
  int writeHex(byte c);
  int writeLine(char * l);
};
#endif


