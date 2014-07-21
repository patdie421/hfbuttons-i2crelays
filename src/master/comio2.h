#ifndef __comio2_h
#define __comio2_h
 
#include <termios.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
 
//#include "error.h"
#include "queue.h"
 
#define COMIO2_MAX_TRAP           16
 
#define COMIO2_ERR_NOERR           0
#define COMIO2_ERR_TIMEOUT         1 // sortie en timeout du sémaphore de synchro
#define COMIO2_ERR_SELECT          2
#define COMIO2_ERR_READ            3
#define COMIO2_ERR_TRAMETYPE       4
#define COMIO2_ERR_UNKNOWN         5
#define COMIO2_ERR_STARTFRAME      6
#define COMIO2_ERR_CHECKSUM        7
#define COMIO2_ERR_BADRESP         8
#define COMIO2_ERR_SYS             9 // erreur systeme
#define COMIO2_ERR_IN_CALLBACK    10
#define COMIO2_ERR_STOPFRAME      11
#define COMIO2_MAX_NB_ERROR       12
 
#define COMIO2_MAX_USER_FRAME_ID 200
#define COMIO2_TRAP_ID           254
 
#define COMIO2_MAX_FRAME_SIZE     80
 
#define COMIO2_TIMEOUT_DELAY       1
#define COMIO2_NB_RETRY            5
 
#define COMIO2_CMD_ERROR           0
#define COMIO2_CMD_READMEMORY      1
#define COMIO2_CMD_WRITEMEMORY     2
#define COMIO2_CMD_CALLFUNCTION    3
 
 
typedef int16_t (*trap_f)(int16_t, char *, int16_t, void *);
 
struct comio2_trap_def_s
{
   trap_f trap;
   void  *userdata;
};
 
typedef struct comio2_ad_s
{
   int             fd;
   pthread_t       read_thread;
   pthread_cond_t  sync_cond;
   pthread_mutex_t sync_lock;
   pthread_mutex_t write_lock;
   pthread_mutex_t ad_lock;
   char            serial_dev_name[255];
   speed_t         speed;
   queue_t         *queue;
   uint16_t        frame_id;
   uint16_t        signal_flag;
  
   struct comio2_trap_def_s
                  tabTraps[COMIO2_MAX_TRAP];
} comio2_ad_t;
 
 
int16_t      comio2_init(comio2_ad_t *ad, char *dev, speed_t speed);
void         comio2_close(comio2_ad_t *ad);
int16_t      comio2_atCmdSendAndWaitResp(comio2_ad_t *ad,
                         char cmd,
                         char *data, // zone donnee d'une trame
                         uint16_t l_data, // longueur zone donnee
                         char *resp,
                         uint16_t *l_resp,
                         int16_t *comio2_err);
int16_t      comio2_atCmdSend(comio2_ad_t *ad,
                         char cmd,
                         char *data, // zone donnee d'une trame
                         uint16_t l_data, // longueur zone donnee
                         int16_t *comio2_err);
void         comoi2_free_ad(comio2_ad_t *ad);
comio2_ad_t *comio2_new_ad();
int16_t      comio2_setTrap(comio2_ad_t *ad, int16_t numTrap, trap_f trap, void *userdata);
int16_t      comio2_removeTrap(comio2_ad_t *ad, uint16_t numTrap);
void         comio2_removeAllTraps(comio2_ad_t *ad);
 
#endif
