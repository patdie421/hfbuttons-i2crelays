#include <stdlib.h>
#include <util/delay.h> // Adds delay_ms and delay_us functions
#include <avr/wdt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <VirtualWire.h>
#include <BlinkLeds.h>
#include <usitwislave.h>


// Rappel : Brochage ATMEL ATTINY45/85 - ARDUINO
//
//                         +-\/-+
// (PCINT5/ADC0) (D5) PB5 1|    |8 Vcc
// (PCINT3/ADC3) (D3) PB3 2|    |7 PB2 (D2) (PCINT2/ADC1/INT0/SCL)
// (PCINT4/ADC2) (D4) PB4 3|    |6 PB1 (D1) (PCINT1/PWM1)
//                    GND 4|    |5 PB0 (D0) (PCINT0/PWM0/SDA)
//                         +----+

// structure d'une trame
typedef struct frame_s {
  uint8_t addr;
  uint8_t rolling;
  uint8_t cmd;
  uint8_t param;
}
__attribute__((packed)) frame_t;

// constante et varables globales
#define EEPROM_ADDR 0 // adresse EEPROM où est stockée l'adresse du module
#define I2C_INIT_ADDR 10
#define INIT_ADDR_PIN PB1
#define SEND_PIN PB3

int8_t input_pins_list[]={
  PB2,PB4,PB0,-1};

static byte net_addr = I2C_INIT_ADDR;
frame_t frame;
volatile boolean pcint_flag = 0;
volatile uint8_t pcIntCurr = 0;
volatile uint8_t pcIntLast = 0xFF;
volatile uint8_t pcIntMask = 0;

BlinkLeds myBlinkLeds(125); // clignotement, changement d'etat toutes les 500 ms


#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


/*
 * Mesure la référence interne à 1v1 de l'ATmega
 */
unsigned int get_1v1_vs_vcc(void)
{
  unsigned int ret;
  uint8_t prr = PRR;

  sbi(PRR, PRADC); // horloge de l'ADC réactivée
  ADMUX = B00001100; // Vcc comme référence de tension et 1v1 (Vbg) comme entrée de l'ADC
  ADCSRA |= (1 << ADEN); // Active le convertisseur analogique -> numérique
  delay(2); // attendre le bon démarrage
  ADCSRA |= (1 << ADSC); // Lance une conversion analogique -> numérique
  while(ADCSRA & (1 << ADSC)); // Attend la fin de la conversion
  ret = ADCH; // Récupère le résultat de la conversion 8 bits seulement
  ADCSRA &= ~(1 << ADEN); // Desactive le convertisseur analogique -> numérique

  PRR = prr; // arret de l'horloge de l'ADC

  return ret;
}


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
  if(input_buffer_length == 3) // potentiellement une demande de changement d'adresse 
  {
    sbi(PORTB, SEND_PIN); // PORTB |= (1 << SEND_PIN);
    if((input_buffer[0]=='@') && (input_buffer[2]=='#')) // demande de changemnet d'adresse ?
    {
      eeprom_write_byte((uint8_t*)EEPROM_ADDR, input_buffer[1]); // ecriture de l'adresse dans l'EEPROM
      while(1){};
    }
  }
}


void init_net_and_addr()
{
  usi_twi_slave(I2C_INIT_ADDR, 0, twi_callback, idle_callback);
}


ISR(PCINT0_vect)
{
  pcint_flag=1;
  pcIntCurr = PINB;
  pcIntMask = pcIntCurr ^ pcIntLast;
  pcIntLast = pcIntCurr;
}


uint8_t pcIntEnable;
void setup()
{
  cli();

  // lecture de l'adresse dans la rom (à l'adresse 0) sans variable intermédiaire pour économiser 1 octet ...
  if((eeprom_read_byte((uint8_t*)0)>10) && (eeprom_read_byte((uint8_t*)0)<255)) // adresse valide ?
  {
    net_addr = eeprom_read_byte((uint8_t*)EEPROM_ADDR);
  }

  if( !(PINB & (1 << INIT_ADDR_PIN)) || (net_addr == I2C_INIT_ADDR) ) // initialisation nécessaire : PB3 sur GND ou addr = 10 ?
  {
    init_net_and_addr();
  }

  // reduction consommation, on desactive tout ce qu'on peut
  ADCSRA &= ~(1<<ADEN); // turn off ADC
  ACSR |= _BV(ACD);     // disable the analog comparator
  PRR = 0;              // tous les timers HS ainsi que les horloges USI et ADC coupés
  DDRB = 0;             // toutes les pattes en entrée
  PORTB = 0x3F;         // pullup sur toutes les pins
  sbi(DDRB, SEND_PIN);  // SEND_PIN en sortie
  cbi(PORTB, SEND_PIN); // SEND_PIN niveau Bas pour démarrer

  // préparation de virtual wire
  vw_set_tx_pin(SEND_PIN);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2048); // Bits per sec
  // initialisation de la trame
  frame.addr=net_addr; // source adresse
  frame.rolling=0; // rolling
  frame.cmd=0; // ordre (push)
  frame.param=0; // param

  // du mask d'interruption "pcint"
  pcIntEnable = 0;
  for(int i=0;input_pins_list[i]!=-1;i++)
    sbi(pcIntEnable, input_pins_list[i]); // mise à jour mask d'interruption "pin change"
  PCMSK = pcIntEnable;

  // interruptions "pin change" activées
  sbi(GIMSK, PCIE); // Turn on Pin Change interrupt
}


void loop()
{
  sei();

  // on va dormir de suite
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // comment on va dormir
  sleep_enable(); // on se prépare à dormir
  sleep_cpu(); // on dort
  sleep_disable(); // on vient d'etre réveillé ...


  cli();
  PCMSK = 0; // plus d'interruption PCINT
  sei();

  _delay_ms(20); // on attend un peu (anti rebond)

  pcIntCurr = PINB;
  // detection des changements d'état
  pcint_flag=0;

  // préparation d'une commande "etat boutons"
  frame.cmd='P'; 
  frame.param=0;

  for(int i=0;input_pins_list[i]!=-1;i++) // on regarde toutes les entrées pour identifier les changements
  {
    // quelles entrees et quels fronts
    if( (pcIntMask & (1 << input_pins_list[i])) )
    {
      if ( pcIntCurr & (1 << input_pins_list[i]) ) // pin & front montant
      {
        sbi(frame.param, i);
      }
      else // front descendant
      {
      }
    }
  }

  if(frame.param)
  {
    for(byte i=0;i<3;i++) // 3 emissions de la meme trame d'affilé
    {
      vw_send((uint8_t *)(&frame), sizeof(frame));
      vw_wait_tx(); // Wait until the whole message is gone
      _delay_ms(i+2);
    }
    frame.rolling++;
  }

  if(!(frame.rolling % 64)) // il est temps de prendre la tension ...
  {
    frame.param = get_1v1_vs_vcc();
    frame.cmd='V';
    vw_send((uint8_t *)(&frame), sizeof(frame));
  }

  _delay_ms(50);

  // retour de l'interruption pcint
  PCMSK = pcIntEnable;
}

