#ifndef __HFBUTTONSI2CRELAYS__
#define __HFBUTTONSI2CRELAYS__

#include "SerialLine.h"
#include "comio2.h"

#define DEBUG 1
#define NB_INTERFACES 62
#define NB_BOUTONS_INTERFACE 3
#define EEPROM_ASSOCS_OFFSET 10
#define EEPROM_NET_ADDR 1
#define DEFAULT_VWRXPIN 7
#define DEFAULT_VWSPEED 2048
#define I2CCONFIGADDR 10


class HFButtonsI2cRelays
{
private:
  struct assoc_button_relay_s {
    uint8_t relay_addr;
    uint8_t relay_num;
  }
  assocs_button_relay[NB_INTERFACES][NB_BOUTONS_INTERFACE];
  uint8_t rollings[NB_INTERFACES];
  uint8_t net;
  uint8_t vwRxPin;
  uint16_t vwSpeed;
  uint8_t hasBegun;
  
  SerialLine *line;
  Comio2 *comio;
  
  void setNet(uint8_t netNum) {
    net = netNum;
  };
  uint8_t getNet() {
    return net;
  };
  uint8_t setRolling(uint8_t addr, uint8_t value) {
    rollings[addr] = value;
  };
  uint8_t getRolling(uint8_t addr) {
    return rollings[addr];
  };
  int8_t addButtonRelay(uint8_t button_addr, uint8_t button_num, uint8_t relay_addr, uint8_t relay_num);
  int8_t delButtonRelay(uint8_t button_addr, uint8_t button_num);
  int getEepromAddr(uint8_t button_addr, uint8_t button_num);
  int8_t loadButtonFromEeprom(uint8_t button_addr, uint8_t button_num);
  int8_t saveButtonToEeprom(uint8_t button_addr, uint8_t button_num);
  int interractiveCmd();
  void HFCmd();

public:
  HFButtonsI2cRelays();
  int8_t init();
  void setVwRxPin(uint8_t pin) {
    vwRxPin = pin;
  };
  void setVwSpeed(uint16_t speed) {
    vwSpeed = speed;
  };
  void begin(SerialLine *l, Comio2 *c);
  
  int8_t loadAllFromEeprom();
  int8_t saveAllToEeprom();
  void run();
  int8_t print(SerialLine *line);
  int pushButton(uint8_t button_addr, uint8_t button_num);
};
#endif


