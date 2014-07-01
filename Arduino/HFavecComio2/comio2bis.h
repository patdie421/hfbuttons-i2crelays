#ifndef Comio2_h
#define Comio2_h

#define __DEBUG

#include <Arduino.h>

//#define _COMIO1_COMPATIBLITY_MODE_

#ifdef _COMIO1_COMPATIBLITY_MODE_
// Taille des mÃ©moires en fonction du type d'arduino (type de microcontroleur)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284P__)
// un MEGA
#define COMIO_MAX_D            54 // nombre max d' E/S logiques
#define COMIO_MAX_A            16 // nombre max d' EntrÃ©es analogiques
#define COMIO_MAX_M            64 // nombre d'octet rÃ©servÃ© pour le partage avec le PC
#elif defined(__AVR_ATmega32u4__)
// un Leonardo ou Ã©quivalant ?
#define COMIO_MAX_D            20
#define COMIO_MAX_A             6
#define COMIO_MAX_M            32
#else
// un UNO ou Ã©quivalant (tous les autres en fait ...)
#define COMIO_MAX_D            14
#define COMIO_MAX_A             6
#define COMIO_MAX_M            32
#endif

#define COMIO_MAX_FX            8 // nombre maximum de fonctions dÃ©clarables

// code opÃ©ration
#define OP_READ                 0
#define OP_WRITE                1
#define OP_FUNCTION             2

// type de variable accessible
#define TYPE_DIGITAL            0
#define TYPE_ANALOG             1
#define TYPE_DIGITAL_PWM        2
#define TYPE_MEMORY             3
#define TYPE_FUNCTION           4

// codes erreurs
#define COMIO_ERR_NOERR         0
#define COMIO_ERR_ENDFRAME      1
#define COMIO_ERR_STARTFRAME    2
#define COMIO_ERR_TIMEOUT       3
#define COMIO_ERR_SELECT        4
#define COMIO_ERR_READ          5
#define COMIO_ERR_MISMATCH      6
#define COMIO_ERR_SYS           7
#define COMIO_ERR_UNKNOWN      99
#endif

// COMIO2 : version 2 du protocole
#define COMIO2_MAX_FX           8 // nombre maximum de fonctions dÃ©clarables
#define COMIO2_MAX_MEMORY      32
#define COMIO2_MAX_DATA        40 // taille maximum d'une trame de donnÃ©es interractive

#define COMIO2_CMD_ERROR        0
#define COMIO2_CMD_READMEMORY   1
#define COMIO2_CMD_WRITEMEMORY  2
#define COMIO2_CMD_CALLFUNCTION 3

#define COMIO2_TRAP_ID        254

#define COMIO2_ERR_NOERR        0
#define COMIO2_ERR_PARAMS       1
#define COMIO2_ERR_UNKNOWN_FUNC 2
#define COMIO2_ERR_UNKNOWN     99

#ifndef COMIO_MAX_M
#define COMIO_MAX_M             COMIO2_MAX_MEMORY
#endif

//
// DÃ©finition des objets de type comio2
//
#ifdef _COMIO1_COMPATIBLITY_MODE_
typedef int (*callback_f)(int);
#endif
typedef int (*callback2_f)(int, char *, int, void *);

class Comio2
{
public:
  Comio2();

#ifdef _COMIO1_COMPATIBLITY_MODE_
  void init(unsigned char io_defs[][3]);
#endif
  void init();
  int run();

#ifdef _COMIO1_COMPATIBLITY_MODE_
  void setFunction(unsigned char num_function, callback_f function);
  void sendTrap(unsigned char num_trap);
  void sendLongTrap(unsigned char num_trap, char *value, char l_value);
#endif
  void setFunction(unsigned char num_function, callback2_f function);
  int  setMemory(unsigned int addr, unsigned char value);
  int  getMemory(unsigned int addr);
  void setReadF(int (*f)(void)) { 
    readF=f; 
  };
  void setWriteF(int (*f)(char)) { 
    writeF=f; 
  };
  void setAvailableF(int (*f)(void)) { 
    availableF=f; 
  };
  void setFlushF(int (*f)(void)) { 
    flushF=f; 
  };
  void setUserdata(void *ud) { 
    userdata = ud; 
  };
  void sendTrap(unsigned char num_trap, char *data, char l_data);

private:
  // IO
#ifdef _COMIO1_COMPATIBLITY_MODE_
  unsigned char comio_digitals[COMIO_MAX_D];
  unsigned char comio_analogs[COMIO_MAX_A];
#endif
  // memoire "partagÃ©e"
  unsigned char comio_memory[COMIO_MAX_M];

#ifdef _COMIO1_COMPATIBLITY_MODE_
  callback_f comio_functions[COMIO_MAX_FX];
#endif
  callback2_f comio_functionsV2[COMIO2_MAX_FX];

  int (* readF)(void);
  int (* writeF)(char car);
  int (* availableF)(void);
  int (* flushF)(void);
  void *userdata;

  void _comio_debut_trameV2(unsigned char id, unsigned char cmd, int l_data, int *ccheksum);
  void _comio_fin_trameV2(int *cchecksum);
  void _comio_send_errorV2(unsigned char id, unsigned char error);
  int _comio_read_frameV2(unsigned char *id, unsigned char *cmd, char *data, int *l_data);
  int _comio_do_operationV2(unsigned char id,unsigned char cmd, char *data, int l_data);

#ifdef _COMIO1_COMPATIBLITY_MODE_
  void _comio_send_frame(unsigned char op, unsigned char var, unsigned char type, unsigned int val);
  void _comio_send_error_frame(unsigned char nerr);
  int _comio_read_frame(unsigned char *op, unsigned char *var, unsigned char *type, unsigned int *val);
  int _comio_valid_operation(unsigned char op, unsigned char var, unsigned char type, unsigned int val);
  boolean _comio_do_operation(unsigned char op, unsigned char var, unsigned char type, unsigned int val);
#endif
};

#endif


