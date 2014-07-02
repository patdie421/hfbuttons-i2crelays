#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <VirtualWire.h>
#include "BlinkLeds.h"
#include "comio2.h"
#include "HFButtonsI2cRelays.h"

#ifdef USBCON
#define YUNSERIAL1 1
#endif
#define USECOMIO 1

#define TIMEOUT 5000 // 5 secondes pour detecter la connexion Serie USB


static HFButtonsI2cRelays buttonsRelays;
SerialLine line(&Serial);
#ifdef USECOMIO
#ifndef YUNSERIAL1
SoftwareSerial mySerial(10, 11);
#endif
Comio2 comio;
#endif

#ifdef USECOMIO
int comio_serial_read()
{
#ifdef YUNSERIAL1
  return Serial1.read();
#else
  return mySerial.read();
#endif
}


int comio_serial_write(char car)
{
#ifdef YUNSERIAL1
  Serial1.write(car);
#else
  mySerial.write(car);
#endif
  return 0;
}


int comio_serial_available()
{
#ifdef YUNSERIAL1
  return Serial1.available();
#else
  return mySerial.available();
#endif
}


int comio_serial_flush()
{
#ifdef YUNSERIAL1
  Serial1.flush();
#else
  mySerial.flush();
#endif
  return 0;
}
#endif


void setup()
{
  pinMode(13, OUTPUT); // led d'état

  // initialisation démarrage de COMIO2 sur softSerial (UNO et équivalant) ou Serial1 (YUN ou LEONARDO)
#ifdef USECOMIO
#ifdef YUNSERIAL1
  Serial1.begin(115200);
  // Wait for U-boot to finish startup. Consume all bytes until we are done.
  do
  {
    while (Serial1.available() > 0)
      Serial1.read();
    delay(1000);
  }
  while (Serial1.available()>0); 
#else
  mySerial.begin(9600);
#endif
  comio.setReadF(comio_serial_read);
  comio.setWriteF(comio_serial_write);
  comio.setAvailableF(comio_serial_available);
  comio.setFlushF(comio_serial_flush);
#endif

// init com. serie sur port USB pour gestion interractive
  uint16_t cntr=0;
  int8_t serial_init_flag=true;
  Serial.begin(9600);
  while (!Serial) {
    delay(1);
    cntr++;
    if(cntr>TIMEOUT)
    {
      serial_init_flag=false;
      break;
    }
  };
  
  SerialLine *linePtr;
  if(serial_init_flag == true) // connexion USB établie ?
  {
     Serial.setTimeout(25); // timeout apres 25ms pour les Serial.read
#ifdef DEBUG
     Serial.println("Started !!!");
#endif
     linePtr=&line;
  }
  else
    linePtr=NULL;
  
  // démarrage de la centrale
  buttonsRelays.loadAllFromEeprom();
  buttonsRelays.setVwRxPin(DEFAULT_VWRXPIN);
  buttonsRelays.setVwSpeed(DEFAULT_VWSPEED);

#ifdef USECOMIO
  buttonsRelays.begin(linePtr, &comio);
#else
  buttonsRelays.begin(linePtr, NULL);
#endif
}


void loop()
{
  static BlinkLeds myBlinkLeds_500ms(500); // clignotement, changement d'etat toutes les 500 ms

  myBlinkLeds_500ms.run();

  myBlinkLeds_500ms.blink(13);

#ifdef USECOMIO
  comio.run();
#endif
  buttonsRelays.run();
}

