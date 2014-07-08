#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
// #include <EEPROM.h>
#include "usitwislave.h"

#define EEPROM_ADDR 0 // adresse EEPROM où est stockée l'adresse i2c du module
#define I2C_INIT_ADDR 10

#define PULSE_WIDTH 100
#define TYPE_VERSION 0x11 // type = 1, version = 1

// 1 ou 8 Mhz fonctionne (à reverifier)
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
//
// NUM
// Pour SET et RESET
// Numero du relais (0, 1 ou 2)
//

#define RELAYS_SET_PIN_IDX 0
#define RELAYS_RESET_PIN_IDX 1
#define RELAYS_STAT_IDX 2

byte relays[][3]={
  {
    PA0, PA1, 0   }
  , {
    PA2, PA3, 0   }
  , {
    PA5, PA7, 0   }
};

// "millis" de la dernière opération
unsigned long lasts[3]={
  0, 0, 0 };

byte i2c_addr = I2C_INIT_ADDR; // adresse par défaut = équipement pas configuré


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
  void setInterval(unsigned int i) { 
    interval = i; 
  };
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


BlinkLeds myBlinkLeds(500); // clignotement, changement d'etat toutes les 500 ms

void idle_callback()
{
  // quand on ne traite pas de données i2c on fait juste clignoter la led
  myBlinkLeds.run();
  if(!myBlinkLeds.getLedState())
    PORTB &= ~(1 << PB2);
  else
    PORTB |= (1 << PB2);
}


static void twi_callback(uint8_t input_buffer_length,
                        const uint8_t *input_buffer,
                        uint8_t *output_buffer_length,
                        uint8_t *output_buffer)
{
  *output_buffer_length = 0; // rien à répondre par défaut
  if(input_buffer_length)
  {
    if(input_buffer_length == 1 && i2c_addr != I2C_INIT_ADDR) // une commande et pas en mode init
    {
      byte value = input_buffer[0];
      byte param = value & 0x0F;
      byte action = (value & 0xF0) >> 4;
      if(param < 3)
      {
        if(my_diff_millis(lasts[param],millis())>500)
        {
          switch(action)
          {
          case 0: // set
            setRelay(relays, param);
            *output_buffer_length = 1;
            output_buffer[0]=relays[param][RELAYS_STAT_IDX];
            break;
          case 1: // reset
            resetRelay(relays, param);
            *output_buffer_length = 1;
            output_buffer[0]=relays[param][RELAYS_STAT_IDX];
            break;
          case 2: // toggle
            toggleRelay(relays, param);
            *output_buffer_length = 1;
            output_buffer[0]=relays[param][RELAYS_STAT_IDX];
            break;
          case 3: // etat (théorique) du relai
            *output_buffer_length = 1;
            output_buffer[0]=relays[param][RELAYS_STAT_IDX];
            break;
          case 13: // type identification
            *output_buffer_length = 1;
            output_buffer[0]=TYPE_VERSION;
            break;
          default:
            break;
          }
          lasts[param]=millis();
        }
      }
    }
    else if(input_buffer_length == 3) // potentiellement une demande de changement d'adresse 
    {
      PORTB |= (1 << PB2); // LED Allumée en continu
      if((input_buffer[0]=='@') && (input_buffer[2]=='#')) // demande de changemnet d'adresse ?
      {
        if((input_buffer[1] > 0) && (input_buffer[1] < 128) && input_buffer[1] != I2C_INIT_ADDR) // avec une adresse valide ?
        {
          i2c_addr=input_buffer[1];
          // EEPROM.write(EEPROM_ADDR, i2c_addr); // ecriture de l'adresse dans l'EEPROM
          eeprom_write_byte(EEPROM_ADDR, i2c_addr); // ecriture de l'adresse dans l'EEPROM
          _delay_ms(2000); // on attend 2 secondes
          usi_twi_slave_stop(); // on demande l'arrêt de la boucle twi_slave pour pouvoir prendre en compte la nouvelle adresse
        }
      }
    }
  }
}


void setup()
{
  // pour les économies d'energie on desactive la "logistique" analogique.
  ADCSRA &= ~(1<<ADEN); // turn off ADC
  ACSR |= _BV(ACD); // disable the analog comparator
  
  // initialisation des ports
  DDRA |= (B10101111); // //PA0,PA1,PA2,PA3,PA5 et PA7 en sortie, PA4 et PA6 sont utilisés par TinyWireS
  DDRB |= (1 << PB2); //LED d'état (sortie)
  DDRB &= ~(1 << PB1); // jumper de configuration (entrée)

 // lecture de l'adresse I2C en EEPROM
  // byte eeprom_addr = EEPROM.read(EEPROM_ADDR);
  byte eeprom_addr = eeprom_read_byte(EEPROM_ADDR);

  if( (eeprom_addr<=10) || (eeprom_addr>127) || (PINB & (1 << PB1)) )
    i2c_addr = I2C_INIT_ADDR;
  else
    i2c_addr = eeprom_addr;

  // LED d'état ON
  PORTB |= (1 << PB2);

  // reset de tout les relais
  for(int i=0;i<3;i++)
    resetRelay(relays, i);
}


void loop()
{
  if(i2c_addr == I2C_INIT_ADDR)
    myBlinkLeds.setInterval(125);
  else    
    myBlinkLeds.setInterval(500);
  
  usi_twi_slave(i2c_addr, 0, twi_callback, idle_callback);
}

