#include "HFButtonsI2cRelays.h"
#include <EEPROM.h>
#include <Wire.h>
#include <VirtualWire.h>
#include "SerialLine.h"
#include "comio2.h"

// Quelques donc à regarder :

// http://playground.arduino.cc/Hardware/Yun (commande reset-mcu par exemple)

// Serial1 <=> /dev/ttyATH0

// on linino the serial port is attached the the console and the kernel messages are also sent to the serial.
// For proper functioning the console must be disabled editing /etc/inittab
// commenting this line #ttyATH0::askfirst:/bin/ash --login effect of this requires a reboot

// disable kernel message :
// echo 0 > /proc/sys/kernel/printk

// http://linino.org/doku.php?id=wiki:gpio : utilisation possible de handshake pour activer D7 et reset du 32u4

char prompt_str[] = "\nCMD : ";

int8_t i2cScan(SerialLine *line)
{
  byte error, address;
  int nDevices;

  line->writeLine((char *)"\nScanning i2c bus ...\n");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    line->writeHex(address);
    line->writeLine((char *)":");
    if (error == 0)
    {
      line->writeLine((char *)" Ok\n");
      nDevices++;
    }
    else
    {
      line->writeLine((char *)" --\n");
    }
  }
  if (nDevices == 0)
    line->writeLine((char *)"No I2C devices found\n");
  else
    line->writeLine((char *)"done\n");
  return 1;
}


int sendCmdToRelay(uint8_t relay_addr, uint8_t relay_num, uint8_t command)
{
  int state=-1; // unknown state
  uint8_t cmd = command << 4;
  cmd = cmd | (relay_num & 0x0F);
  Wire.beginTransmission(relay_addr);
  Wire.write(cmd);
  Wire.endTransmission();
  
  if(Wire.requestFrom(relay_addr, 1))
  {
     state=Wire.read();
     
     // ici, envoyer un trap
     
     return state;
  }

  return state;
}


int relayGetState(uint8_t relay_addr, uint8_t relay_num)
{
  int state=0;
  
  uint8_t cmd = 3 << 4;
  cmd = cmd | (relay_num & 0x0F);
  
  Wire.beginTransmission(relay_addr);
  Wire.write(cmd);
  Wire.endTransmission();
  
  if(Wire.requestFrom(relay_addr, 1))
  {
     state=Wire.read();
     return state;
  }
  return -1;
}


int comioCallback(int id, char *data, int l_data, void *userdata)
{
  HFButtonsI2cRelays *buttonsRelays = (HFButtonsI2cRelays *)userdata;
  uint8_t addr, num, action;
  int flag = 0;
  action = data[0];
  addr = data[1];
  num = data[2];
  
  switch(action)
  {
    case 'T':
      // envoyer la commande "toggle" au relais
      flag=sendCmdToRelay(addr, num, 2);
      break;
    case 'S':
      flag=sendCmdToRelay(addr, num, 0);
      break;
    case 'R':
      flag=sendCmdToRelay(addr, num, 1);
      break;
    case 'P':
      if(buttonsRelays)
        flag=buttonsRelays->pushButton(addr, num);
      break;
    default:
      break;
  }
  return flag;
}


int comioCallback2(int id, char *data, int l_data, void *userdata)
{
  HFButtonsI2cRelays *buttonsRelays = (HFButtonsI2cRelays *)userdata;
  int16_t state;
  
  if(l_data == 2)
  {
     state = relayGetState(data[0], data[1]);
  }
  else
     state=-1;

  return state;
}


HFButtonsI2cRelays::HFButtonsI2cRelays()
{
  vwRxPin = DEFAULT_VWRXPIN;
  vwSpeed = DEFAULT_VWSPEED;
  hasBegun = false;
  comio = NULL;
}


int HFButtonsI2cRelays::getEepromAddr(uint8_t button_addr, uint8_t button_num)
{
  if((button_addr <= NB_INTERFACES) && (button_num <= NB_BOUTONS_INTERFACE))
  {
    byte *sAddr = (byte *)(&assocs_button_relay[0][0]);
    byte *bAddr = (byte *)(&assocs_button_relay[button_addr][button_num]);
    int eepromAddr = (int)(bAddr - sAddr) + EEPROM_ASSOCS_OFFSET;
    return eepromAddr;
  }
  return -1;
}


int8_t HFButtonsI2cRelays::init()
{
  for(int i=0;i<NB_INTERFACES;i++)
  {
    for(int j=0;j<NB_BOUTONS_INTERFACE;j++)
    {
      assocs_button_relay[i][j].relay_addr=255;
      assocs_button_relay[i][j].relay_num=255;
    }
    rollings[i]=0;
  }
  net=255;
  for(int i=0;i<1024;i++) // effacement de toutes l'eeprom
    EEPROM.write(i,255);
  return 1;
}


int8_t HFButtonsI2cRelays::addButtonRelay(uint8_t button_addr, uint8_t button_num, uint8_t relay_addr, uint8_t relay_num)
{
  if((button_addr <= NB_INTERFACES) && (button_num <= NB_BOUTONS_INTERFACE))
  {
    if((relay_addr < 128 && relay_num < 3) || (relay_addr == 254))
    {
      assocs_button_relay[button_addr][button_num].relay_addr=relay_addr;
      assocs_button_relay[button_addr][button_num].relay_num=relay_num;
    }
    else
      return 0;
    saveButtonToEeprom(button_addr, button_num);
    return 1;
  }
  else
    return 0;
}


int8_t HFButtonsI2cRelays::delButtonRelay(uint8_t button_addr, uint8_t button_num)
{
  if((button_addr <= NB_INTERFACES) && (button_num <= NB_BOUTONS_INTERFACE))
  {
    assocs_button_relay[button_addr][button_num].relay_addr=255;
    assocs_button_relay[button_addr][button_num].relay_num=255;
    saveButtonToEeprom(button_addr, button_num);
    return 1;
  }
  else
    return 0;
}


int8_t HFButtonsI2cRelays::loadAllFromEeprom()
{
  for(int i=0;i<NB_INTERFACES;i++)
    for(int j=0;j<NB_BOUTONS_INTERFACE;j++)
      loadButtonFromEeprom(i, j);
  uint8_t net = EEPROM.read(EEPROM_NET_ADDR);
  if(net < 4)
    setNet(EEPROM.read(EEPROM_NET_ADDR));
  else
    setNet(255);
  return 1;
}


int8_t HFButtonsI2cRelays::saveAllToEeprom()
{
  for(int i=0;i<NB_INTERFACES;i++)
    for(int j=0;j<NB_BOUTONS_INTERFACE;j++)
      saveButtonToEeprom(i, j);
  EEPROM.write(EEPROM_NET_ADDR, net);
  return 1;
}


int8_t HFButtonsI2cRelays::loadButtonFromEeprom(uint8_t button_addr, uint8_t button_num)
{
  int addr = getEepromAddr(button_addr, button_num);
  assocs_button_relay[button_addr][button_num].relay_addr = EEPROM.read(addr);
  assocs_button_relay[button_addr][button_num].relay_num = EEPROM.read(addr + 1);
  return 1;
}


int8_t HFButtonsI2cRelays::saveButtonToEeprom(uint8_t button_addr, uint8_t button_num)
{
  int addr = getEepromAddr(button_addr, button_num);
  EEPROM.write(addr, assocs_button_relay[button_addr][button_num].relay_addr);
  EEPROM.write(addr + 1, assocs_button_relay[button_addr][button_num].relay_num);
  return 1;
}


int8_t HFButtonsI2cRelays::print(SerialLine *line)
{
  line->writeLine((char *)"\n$NET$\n");
  line->writeByte((byte)getNet());
  line->writeLine((char *)"\n#NET#\n");
  line->writeLine((char *)"$LIST$\n");
  for(int i=0;i<NB_INTERFACES;i++)
  {
    for(int j=0;j<NB_BOUTONS_INTERFACE;j++)
    {
      if(assocs_button_relay[i][j].relay_addr != 255)
      {
        line->writeByte((byte)i);
        line->writeLine((char *)";");
        line->writeByte((byte)j);
        line->writeLine((char *)";");
        line->writeByte((byte)assocs_button_relay[i][j].relay_addr);
        line->writeLine((char *)";");
        line->writeByte((byte)assocs_button_relay[i][j].relay_num);
        line->writeLine((char *)"\n");
      }
    }
  }
  line->writeLine((char *)"#LIST#");
  return 1;
}


int HFButtonsI2cRelays::pushButton(uint8_t button_addr, uint8_t button_num)
{
  uint8_t relay_addr;
  uint8_t relay_num;
  
  if(button_addr <= NB_INTERFACES && button_num <= NB_BOUTONS_INTERFACE)
  {
    relay_num=assocs_button_relay[button_addr][button_num].relay_num;
    relay_addr=assocs_button_relay[button_addr][button_num].relay_addr;
    if(relay_addr == 255) // pas de relais associé au bouton
      return 0;
    if(relay_addr == 254) // relay virtuel associé au bouton
    {
#ifdef DEBUG
      Serial.print("VIRUAL_RELAY : ");
      Serial.println(relay_num);
#endif

      // ici, envoyer un trap
      
      return 1;
    }
    else
    {
      int state = sendCmdToRelay(relay_addr, relay_num, 2);

      return 1;
    }
  }
  return 0;
}


void HFButtonsI2cRelays::HFCmd()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    if(buflen == 4)
    {
      uint8_t net = (buf[0] & B11000000) >> 6;
      uint8_t addr = buf[0] & B00111111;
      if(net == getNet())
      {
        if(buf[1] != getRolling(addr))
        {
          if(buf[2] == 'V') // etat pile
          {
#ifdef DEBUG
            Serial.print("V=");
            Serial.print(buf[3]);
            Serial.print(prompt_str);
#endif
            // + envoyer d'un trap COMIO2 ici
            if(comio)
            {
            }
          }
          else if(buf[2] == 'P') // action bouton
          {
            uint8_t etat = buf[3] & 0x0F;
            for(uint8_t i = 0; i<4; i++)
            {
              if(etat & (1 << i))
              {
#ifdef DEBUG
                Serial.print("\nBUTTON #");
                Serial.print(i);
                Serial.print("/");
                Serial.print(addr);
                Serial.print("/");
                Serial.print(net);
                Serial.print(" PUSHED");
                Serial.print(prompt_str);
#endif
                pushButton(addr, i);
                // + envoyer un trap COMIO2 ici
                if(comio)
                {
                }
              }
            }
            setRolling(addr,buf[1]);
          }
        }
      }
    }
  }
}


int HFButtonsI2cRelays::interractiveCmd()
{
  uint8_t flag = 0;

  if(!line)
    return -1;  
  if(line->available())
  {
    int a; // variable temporaire pour stockage intermédiaire de c;
    uint8_t button_addr, button_num, relay_addr, relay_num;
    int c = line->readByte();
    if(c == '$')
    {
      c=line->readByte();
      switch(c)
      {
        // commandes sans paramètres
      case 'L': // list
      case 'W': // liste les adresses i2c utilisées
      case 'I': // réinitialisation totale
        a=c;
        if(line->readByte()!='#')
          break;
        switch(a)
        {
        case 'L':
          flag=print(line);
          break;
        case 'W':
          flag=i2cScan(line);
          break;
        case 'I': // reinitialisation complète du paramétrage
          flag=init();
          break;
        }
        break;
        // commande avec 1 seul parametre
      case 'N': // choix de l'adresse du réseau de boutons
      case 'K': // change addr for default 10 $K:20 => adresse d'initialisation 10 vers 20
        a=c;
        if(line->readByte()!=':')
          break;
        c=line->parseInt();
        if(c < 0 || line->readByte()!='#')
          break;
        switch(a)
        {
        case 'N':
          if(c > 3)
            break;
          setNet(c);
          EEPROM.write(EEPROM_NET_ADDR, (char)c);
          break;
        case 'K': // change addr for default 10 $K:20 => adresse d'initialisation 10 vers 20
          if(c < 1 || c > 127)
            break;
          Wire.beginTransmission(I2CCONFIGADDR);
          Wire.write('@');
          Wire.write(c);
          Wire.write('#');
          Wire.endTransmission();
          break;
        }
        flag=1;
        break;
      case 'A': // ajout/modification d'une association. Trame du type $A:50;2;12;2#
        if(line->readByte()!=':')
          break;
        c=line->parseInt();
        if(c < 0 || c > NB_INTERFACES || line->readByte()!=';')
          break;
        button_addr = c;
        c=line->parseInt();
        if(c < 0 || c > 2 || line->readByte()!=';')
          break;
        button_num=c;
        c=line->parseInt();
        if(c < 0 || c > 127 || line->readByte()!=';')
          break;
        relay_addr=c;
        c=line->parseInt();
        if(c < 0 || c > 2 || line->readByte()!='#')
          break;
        relay_num=c;
        flag = addButtonRelay(button_addr, button_num, relay_addr, relay_num);
        break;
        // commande avec 2 parametres (addr/num button)
      case 'D': // suppression d'une association. Trame du type $D:50;2#
      case 'P': // simulation appuie sur un bouton
        a=c;
        if(line->readByte()!=':')
          break;
        c=line->parseInt();
        if(c < 0 || c > NB_INTERFACES || line->readByte()!=';')
          break;
        button_addr=c;
        c=line->parseInt();
        if(c > 2 || line->readByte()!='#')
          break;
        button_num=c;
        switch(a)
        {
        case 'D':
          flag=delButtonRelay(button_addr, button_num);
          break;
        case 'P':
          flag=pushButton(button_addr, button_num);
          break;
        }
        break;
      case 'T': // action relais : toggle. Trame du type $T:12;2#
      case 'S': // action relais : set. Trame du type $S:12;2#
      case 'R': // action relais : reset. Trame du type $R:12:2#
        a=c;
        if(line->readByte()!=':')
          break;
        c=line->parseInt();
        if(c < 1 || c > 127 || line->readByte()!=';')
          break;
        relay_addr=c;
        c=line->parseInt();
        if(c > 2 || line->readByte()!='#')
          break;
        relay_num=c;
        switch(a)
        {
        case 'T':
          // envoyer la commande "toggle" au relais
          flag=sendCmdToRelay(relay_addr, relay_num, 2);
          break;
        case 'S':
          flag=sendCmdToRelay(relay_addr, relay_num, 0);
          break;
        case 'R':
          flag=sendCmdToRelay(relay_addr, relay_num, 1);
          break;
        }
        break;
      case 'J': // ajout/modification d'une association. Trame du type $A:50;2;12;2#
        if(line->readByte()!=':')
          break;
        c=line->parseInt();
        if(c < 1 || c > 127 || line->readByte()!=';')
          break;
        relay_addr=c;
        c=line->parseInt();
        if(c < 1 || c > 127 || line->readByte()!='#')
          break;
        Wire.beginTransmission(relay_addr);
        Wire.write('@');
        Wire.write(c);
        Wire.write('#');
        Wire.endTransmission();
        flag=1;
        break;
      default:
        flag=2;
        break;
      }
    }
    else
      flag=3;
  }
  return flag;
}


void HFButtonsI2cRelays::run()
{
  if(hasBegun)
  {
    HFCmd();
    
    if(line)
    {
      int n = line->readLine(); // une ligne disponible ?
      if(n>0)
      {
        int flag=interractiveCmd(); // traitement de la ligne
        if(flag == 0)
           line->writeLine((char *)"\nSYNTAX OR PARAMETER(S) ERROR");
        else if(flag == 1)
           line->writeLine((char *)"\nOK");
        else if(flag == 2)
          line->writeLine((char *)"\nUNKNOWN COMMAND");
        else if(flag == -1)
          return;
        else
          line->writeLine((char *)"\n???");
        line->flush(); // on vide le buffer pour repartir à 0.
        line->writeLine(prompt_str);
      }
      else if(n<0)
      {
        line->writeLine((char *)"\nOverflow !!!\n");
      }
    }
  }
}


void HFButtonsI2cRelays::begin(SerialLine *l, Comio2 *c)
{
  hasBegun = true;

  vw_set_rx_pin(vwRxPin);
  vw_setup(vwSpeed);
  vw_rx_start();

  line = l;
  if(line)
    line->writeLine(prompt_str);

  comio = c;
  if(comio)
  {
    comio->setFunction(1, comioCallback);
    comio->setFunction(2, comioCallback2);
    comio->setUserdata(this);
  }
  
    // init com. I2C pour le pilotage des relays
  Wire.begin();
}

