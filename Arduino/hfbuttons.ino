#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <EEPROM.h>
#include "TinyWireS.h"

//#define MYDEBUG
#ifdef MYDEBUG
#include <TinyDebugSerial.h>
#endif

#define Reset_AVR() cli(); wdt_enable(WDTO_15MS); while(1){}

#define I2C_INIT_ADDR   10
#define PULSE_WIDTH    100
#define TYPE_VERSION  0x11 // type = 1, version = 1

// 1 ou 8 Mhz fonctionne
// Ordre IC2 : 1 octet
// B7  B6  B5  B4  B3  B2  B1  B0
//!--------------!!--------------!
// ACT NUM
//
// ACT :
// 0 : SET
// 1 : RESET
// 2 : TOGGLE
// 3 : etat (théorique) du relai
// 13: type identification
// 14: AVR reset
// 15: changement d'adresse
//
// NUM
// Pour SET et RESET
// Numero du relais (0, 1 ou 2)
//

#define RELAYS_SET_PIN_IDX   0
#define RELAYS_RESET_PIN_IDX 1
#define RELAYS_STAT_IDX      2

byte relays[][3]={
  {
    PA0, PA1, 0    }
  , {
    PA2, PA3, 0    }
  , {
    PA5, PA7, 0    }
};

unsigned long lasts[3]={ 
  0, 0, 0 };

byte addr = 10; // adresse par défaut = équipement pas configuré


/*
 * différence "juste" entre deux chronos (tient compte d'un éventuel passage par 0 on est pas a l'abrit ...)
 */
inline unsigned long my_diff_millis(unsigned long chrono, unsigned long now)
{
  return now >= chrono ? now - chrono : 0xFFFFFFFF - chrono + now;
}


/* pour un clignotement de led synchronisé */
class BlinkLeds
{
public:
  BlinkLeds(unsigned int i);
  void run();
  int getLedState() {
    return ledState;
  }
private:
  unsigned int interval;
  int ledState;
  unsigned long previousMillis;
};


/*
 * Création d'un clignotement de led
 */
BlinkLeds::BlinkLeds(unsigned int i)
{
  interval = i;
  ledState = LOW;
  previousMillis = 0;
}


/*
 * mis à jour de l'état d'un clignotement en fonction du temps passé
 */
void BlinkLeds::run()
{
  unsigned long currentMillis = millis();
  if(my_diff_millis(previousMillis, currentMillis) > interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
  }
}


int setRelay(byte relays[][3], byte num)
{
  if(num < 3)
  {
    PORTA &= ~(1 << relays[num][RELAYS_RESET_PIN_IDX]); //digitalWrite(relays[param][1], LOW);
    PORTA |= (1 << relays[num][RELAYS_SET_PIN_IDX]); //digitalWrite(relays[param][0], HIGH);
    _delay_ms(PULSE_WIDTH);
    PORTA &= ~(1 << relays[num][RELAYS_SET_PIN_IDX]); //digitalWrite(relays[param][0], LOW);
    relays[num][RELAYS_STAT_IDX]=HIGH;
    return -1;
  }
  else
    return 0;
}


int resetRelay(byte relays[][3], byte num)
{
  if(num < 3)
  {
    PORTA &= ~(1 << relays[num][RELAYS_SET_PIN_IDX]); //digitalWrite(relays[param][0], LOW);
    PORTA |= (1 << relays[num][RELAYS_RESET_PIN_IDX]); //digitalWrite(relays[param][1], HIGH);
    _delay_ms(PULSE_WIDTH);
    PORTA &= ~(1 << relays[num][RELAYS_RESET_PIN_IDX]); //digitalWrite(relays[param][1], LOW);
    relays[num][RELAYS_STAT_IDX]=LOW;
    return -1;
  }
  else
    return 0;
}


int toggleRelay(byte relays[][3], byte num)
{
  if(num < 3)
  {
    if(relays[num][RELAYS_STAT_IDX]==LOW)
      setRelay(relays, num);
    else
      resetRelay(relays, num);
    return -1;
  }
  else
    return 0;
}


/*
 * initialisation de l'adresse i2c de l'attiny
 */
void init_addr()
{
  BlinkLeds myBlinkLeds_125ms(125); // clignotement rapide pour signaler la procédure d'init en cours

  // reset de tout les relais
  for(int i=0;i<3;i++)
    resetRelay(relays, i);

  for(;;)
  {
    // clignotement rapide de la led d'état
    myBlinkLeds_125ms.run();
    if(!myBlinkLeds_125ms.getLedState())
      PORTB &= ~(1 << PB2);
    else
      PORTB |= (1 << PB2);

    // lecture d'une séquence @<addr># avec <addr> > 0 et <addr> < 128
    if (TinyWireS.available())
    {
      if(TinyWireS.receive() == '@') // premier caractère de la séquence
      {
        byte addr = TinyWireS.receive(); // deuxième caractère
        if(addr > 0 && addr < 128) // est il valide ?
        { // oui, lecture du dernier caractere de la séquence
          if(TinyWireS.receive()=='#') // dernier caractère de la séquence valide ?
          {
            // Oui, allumage de la led en continu
            PORTB |= (1 << PB2);
            _delay_ms(2000); // on attend 2 secondes
            EEPROM.write(0,addr); // ecriture de l'adresse dans l'EEPROM
            Reset_AVR();
            return;
          }
        }
      }
    }
  }
}

#ifdef MYDEBUG
TinyDebugSerial mySerial = TinyDebugSerial();
#endif

void setup()
{
  // pour les économies d'energie.
  ADCSRA &= ~(1<<ADEN); // turn off ADC
  ACSR |= _BV(ACD);     // disable the analog comparator

  // initialisation des ports
  DDRA |= (B10101111); // //PA0,PA1,PA2,PA3,PA5 et PA7 en sortie, PA4 et PA6 sont utilisés par TinyWireS
  DDRB |= (1 << PB2); //LED d'état (sortie)
  DDRB &= ~(1 << PB1); // jumper de configuration (entrée)

  byte eeprom_addr = EEPROM.read(0); // lecture de l'adresse en EEPROM

  if((eeprom_addr>10) && (eeprom_addr<128)) // adresse valide ?
  {
    addr = eeprom_addr;
  }

  if( (PINB & (1 << PB1)) || (addr == 10) ) // initialisation nécessaire ?
  {
    TinyWireS.begin(I2C_INIT_ADDR); // adresse d'initialisation : 10
    init_addr();
  }

  // reset de tout les relais
  for(int i=0;i<3;i++)
    resetRelay(relays, i);

  // inititialisation de l'adresse i2c définitive
  TinyWireS.begin(addr);

#ifdef MYDEBUG
  mySerial.begin(9600);
  mySerial.print("SetResetIC2Relay debug mode\n");
#endif
}

BlinkLeds myBlinkLeds_500ms(500); // clignotement, changement d'etat toutes les 500 ms

void loop()
{
  myBlinkLeds_500ms.run();

  if(!myBlinkLeds_500ms.getLedState())
    PORTB &= ~(1 << PB2);
  else
    PORTB |= (1 << PB2);

  if (TinyWireS.available())
  {
    byte value = TinyWireS.receive();
    byte param = value & 0x0F;
    byte action = (value & 0xF0) >> 4;

#ifdef MYDEBUG
    mySerial.print(value);
    mySerial.print(":");
    mySerial.print(action);
    mySerial.print("-");
    mySerial.print(param);
    mySerial.print("\n");
#endif

    if(param < 3)
    {
      if(my_diff_millis(lasts[param],millis())>500)
      {
        switch(action)
        {
        case 0: // set
          setRelay(relays, param);
          break;
        case 1: // reset
          resetRelay(relays, param);
          break;
        case 2: // toggle
          toggleRelay(relays, param);
          break;
        case 3: // etat (théorique) du relai
          TinyWireS.send(relays[param][RELAYS_STAT_IDX]);
          break;
        case 13: // type identification
          TinyWireS.send(TYPE_VERSION);
          break;
        case 14: // AVR reset
          Reset_AVR();
          break;
        case 15: // changement d'adresse
          init_addr();
          break;
        default:
          break;
        }
        lasts[param]=millis();
      }
    }
  }
}

