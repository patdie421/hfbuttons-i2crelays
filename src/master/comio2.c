//
//  comio2.c
//  Comio2
//
//  Created by Patrice Dietsch on 23/06/2014.
//  Copyright (c) 2014 Patrice Dietsch. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "comio2.h"
#include "debug.h"

typedef struct comio2_queue_elem_s
{
   char frame[255];
   uint16_t l_frame;
   uint32_t tsp;
   int16_t comio2_err;
   
} comio2_queue_elem_t;


void     _comio2_free_queue_elem(void *d); // pour vider_file2
int16_t  _comio2_read_frame(int fd, char *frame, uint16_t *l_frame, int16_t *nerr);
int16_t  _comio2_build_frame(char id, char *frame, char *cmd, uint16_t l_cmd);
int16_t  _comio2_write_frame(int fd, char id, char *cmd, uint16_t l_cmd, int16_t *nerr);
void     _comio2_flush_old_responses_queue(comio2_ad_t *ad);
int16_t  _comio2_add_response_to_queue(comio2_ad_t *ad, char *frame, uint16_t l_frame);
uint32_t _comio2_get_timestamp();
void    *_comio2_thread(void *args);


uint16_t _comio2_get_frame_data_id(comio2_ad_t *ad)
/**
 * \brief     retour un identifiant "unique" à utiliser pour construire une de trame API AT.
 * \details   Les identifiants sont séquentiels et se trouvent dans la plage 1 - COMIO2_MAX_USER_FRAME_ID. Les id > COMIO2_MAX_USER_FRAME_ID sont resevés pour d'autres utilisations.
 * \param     ad     descripteur comio.
 * \return    id     entre 1 et COMIO2_MAX_USER_FRAME_ID
 */
{
   uint16_t ret;
   
   pthread_cleanup_push( (void *)pthread_mutex_unlock, (void *)&(ad->ad_lock) );
   pthread_mutex_lock(&(ad->ad_lock));
   
   ret=ad->frame_id;
   
   ad->frame_id++;
   if (ad->frame_id>COMIO2_MAX_USER_FRAME_ID)
   ad->frame_id=1;
   
   pthread_mutex_unlock(&(ad->ad_lock));
   pthread_cleanup_pop(0);
   
   return ret;
}


int16_t comio2_init(comio2_ad_t *ad, char *dev, speed_t speed)
/**
 * \brief     Initialise les mécanismes de communication avec un periphérique serie COMIO
 * \details   Cette fonction assure :
 *            - l'initialisation du "descripteur" (ensemble des éléments nécessaire à la gestion des échanges avec un périphérique COMIO). Le descripteur sera utilisé par toutes les fonctions liées à la gestion COMIO
 *            - le parametrage et l'ouverture du port serie (/dev/ttyxxx)
 * \param     ad     descripteur à initialiser. Il doit etre alloue par l'appelant.
 * \param     dev    le chemin "unix" (/dev/ttyxxx) de l'interface série (ou USB)
 * \param     speed  le debit de la liaison serie (constante de termios)
 * \return    -1 en cas d'erreur, descripteur du périphérique sinon
 */
{
   struct termios options, options_old;
   int fd;
   //   int16_t nerr;
   
   // ouverture du port
   int flags;
   
   memset (ad,0,sizeof(comio2_ad_t));
   
   flags=O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
#ifdef O_CLOEXEC
   flags |= O_CLOEXEC;
#endif
   
   fd = open(dev, flags);
   if (fd == -1)
   {
      // ouverture du port serie impossible
      return -1;
   }
   strcpy(ad->serial_dev_name,dev);
   ad->speed=speed;
   
   // sauvegarde des caractéristiques du port serie
   tcgetattr(fd, &options_old);
   
   // initialisation à 0 de la structure des options (termios)
   memset(&options, 0, sizeof(struct termios));
   
   // paramétrage du débit
   if(cfsetispeed(&options, speed)<0)
   {
      // modification du debit d'entrée impossible
      return -1;
   }
   if(cfsetospeed(&options, speed)<0)
   {
      // modification du debit de sortie impossible
      return -1;
   }
   
   // ???
   options.c_cflag |= (CLOCAL | CREAD); // mise à 1 du CLOCAL et CREAD
   
   // 8 bits de données, pas de parité, 1 bit de stop (8N1):
   options.c_cflag &= ~PARENB; // pas de parité (N)
   options.c_cflag &= ~CSTOPB; // 1 bit de stop seulement (1)
   options.c_cflag &= ~CSIZE;
   options.c_cflag |= CS8; // 8 bits (8)
   
   // bit ICANON = 0 => port en RAW (au lieu de canonical)
   // bit ECHO =   0 => pas d'écho des caractères
   // bit ECHOE =  0 => pas de substitution du caractère d'"erase"
   // bit ISIG =   0 => interruption non autorisées
   options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
   
   // pas de contrôle de parité
   options.c_iflag &= ~INPCK;
   
   // pas de contrôle de flux
   options.c_iflag &= ~(IXON | IXOFF | IXANY);
   
   // parce qu'on est en raw
   options.c_oflag &=~ OPOST;
   
   // VMIN : Nombre minimum de caractère à lire
   // VTIME : temps d'attentes de données (en 10eme de secondes)
   // à 0 car O_NDELAY utilisé
   options.c_cc[VMIN] = 0;
   options.c_cc[VTIME] = 0;
   
   // réécriture des options
   tcsetattr(fd, TCSANOW, &options);
   
   // préparation du descripteur
   ad->fd=fd;
   
   // préparation synchro consommateur / producteur
   pthread_cond_init(&ad->sync_cond, NULL);
   pthread_mutex_init(&ad->sync_lock, NULL);
   
   // verrou de mutex écriture vers mcu
   pthread_mutex_init(&ad->write_lock, NULL);

// verrou de section critique interne
   pthread_mutex_init(&ad->ad_lock, NULL);
   
   ad->queue=(queue_t *)malloc(sizeof(queue_t));
   if(!ad->queue)
   return -1;
   
   init_queue(ad->queue); // initialisation de la file
   
   ad->frame_id=1;
   ad->signal_flag=0;
   
   if(pthread_create (&(ad->read_thread), NULL, _comio2_thread, (void *)ad))
   return -1;
   
   return fd;
}


comio2_ad_t *comio2_new_ad()
{
   return (comio2_ad_t *)malloc(sizeof(comio2_ad_t));
}


void comio2_clean_ad(comio2_ad_t *ad)
/**
 * \brief     Vide les "conteneurs" d'un descipteur ad
 * \details   Appeler cette fonction pour libérer la mémoire allouée pour un descipteur de communication comio.
 * \param     ad   descripteur de communication comio
 */
{
   if(ad)
   {
      if(ad->queue)
      {
         clear_queue(ad->queue,_comio2_free_queue_elem);
         free(ad->queue);
         ad->queue=NULL;
      }
   }
}


void comio2_free_ad(comio2_ad_t *ad)
/**
 * \brief     Vide les "conteneurs" d'un descipteur ad et libère la mémoire
 * \details   Appeler cette fonction pour libérer la mémoire allouée pour un descipteur de communication comio.
 * \param     ad   descripteur de communication comio
 */
{
   if(ad)
   {
      comio2_clean_ad(ad);
      free(ad);
   }
}


void comio2_close(comio2_ad_t *ad)
{
   /**
    * \brief     fermeture d'une communication avec un xbee
    * \details   arrête le thead de gestion de la communication, ménage dans ad et fermeture du "fichier".
    * \param     ad   descripteur de communication comio
    */
   pthread_cancel(ad->read_thread);
   pthread_join(ad->read_thread, NULL);
   comio2_free_ad(ad);
   close(ad->fd);
}


int16_t comio2_atCmdSend(comio2_ad_t *ad,
                         char cmd,
                         char *data, // zone donnee d'une trame
                         uint16_t l_data, // longueur zone donnee
                         int16_t *comio2_err)
{
   int16_t nerr;
   
   if(pthread_self()==ad->read_thread) // risque de dead lock si appeler par un call back => on interdit
   {
      nerr=COMIO2_ERR_IN_CALLBACK;
      return -1;
   }
   
   uint16_t frame_data_id=0; // 0 = pas de réponse attendues
   char cmd_data[255];
   uint16_t l_cmd_data;
   
   cmd_data[0]=cmd;
   for(int i=0;i<l_data;i++)
   cmd_data[i+1]=data[i];
   l_cmd_data=l_data+1;
   
   int ret;
   pthread_cleanup_push( (void *)pthread_mutex_unlock, (void *)&(ad->write_lock) );
   pthread_mutex_lock(&ad->write_lock);
   ret=_comio2_write_frame(ad->fd, frame_data_id, cmd_data, l_cmd_data, &nerr);
   pthread_mutex_unlock(&ad->write_lock);
   pthread_cleanup_pop(0);

   if(ret==0) // envoie de l'ordre
   {
      return 0;
   }
   return 1;
}


int16_t comio2_atCmdSendAndWaitResp(comio2_ad_t *ad,
                                    char cmd,
                                    char *data, // zone donnee d'une trame
                                    uint16_t l_data, // longueur zone donnee
                                    char *resp,
                                    uint16_t *l_resp,
                                    int16_t *comio2_err)
/**
 * \brief     Transmet une commande
 * \details
 * \param     ad           descripteur de communication
 * \param     frame_data   ensemble de la commande à transmettre.
 * \param     l_frame_data longueur de la commande.
 * \param     resp         ensemble de la réponse.
 * \param     l_resp       longueur de la réponse.
 * \param     comio2_err     numero d'erreur
 * \return    0 = OK, -1 = KO, voir nerr pour le type d'erreur.
 */
{
   comio2_queue_elem_t *e;
   int16_t nerr;
   int16_t return_val=1;
   
   if(pthread_self()==ad->read_thread) // risque de dead lock si appeler par un call back => on interdit
   {
      nerr=COMIO2_ERR_IN_CALLBACK;
      return -1;
   }
   
   // consolidation des données
   char cmd_data[255];
   uint16_t l_cmd_data;
   
   cmd_data[0]=cmd;
   for(uint16_t i=0;i<l_data;i++)
   cmd_data[i+1]=data[i];
   l_cmd_data=l_data+1;
   
   // construction de la trame de la zone data et d'un identifiant de trame "unique"
   int16_t frame_data_id=_comio2_get_frame_data_id(ad);
   
   // on envoie la demande
   int ret;
   pthread_cleanup_push( (void *)pthread_mutex_unlock, (void *)&(ad->write_lock) );
   pthread_mutex_lock(&ad->write_lock);
   ret=_comio2_write_frame(ad->fd, frame_data_id, cmd_data, l_cmd_data, &nerr);
   pthread_mutex_unlock(&ad->write_lock);
   pthread_cleanup_pop(0);

   if(ret==0) // envoie de l'ordre
   {
      int16_t ret;
      int16_t boucle=COMIO2_NB_RETRY; // 5 tentatives de 1 secondes
      uint16_t notfound=0;
      do
      {
         // on va attendre le retour dans la file des reponses
         pthread_cleanup_push( (void *)pthread_mutex_unlock, (void *)&(ad->sync_lock) );
         pthread_mutex_lock(&(ad->sync_lock));
         if(ad->queue->nb_elem==0 || notfound==1)
         {
            // rien a lire => on va attendre que quelque chose soit mis dans la file
            struct timeval tv;
            struct timespec ts;
            gettimeofday(&tv, NULL);
            ts.tv_sec = tv.tv_sec + COMIO2_TIMEOUT_DELAY;
            ts.tv_nsec = 0;
            
            ret=pthread_cond_timedwait(&ad->sync_cond, &ad->sync_lock, &ts);
            if(ret)
            {
               if(ret!=ETIMEDOUT)
               {
                  *comio2_err=COMIO2_ERR_SYS;
                  return_val=-1;
                  goto next_or_return;
               }
            }
         }
         
         // a ce point il devrait y avoir quelque chose dans la file.
         if(first_queue(ad->queue)==0) // parcours de la liste jusqu'a trouver une reponse pour nous
         {
            do
            {
               if(current_queue(ad->queue, (void **)&e)==0)
               {
                  if((uint16_t)(e->frame[0] & 0xFF)==frame_data_id) // le premier octet d'une reponse doit contenir le meme id que celui de la question
                  {
                     if(e->comio2_err!=COMIO2_ERR_NOERR) // la reponse est une erreur
                     {
                        *comio2_err=e->comio2_err; // on la retourne directement
                        
                        // et on fait le menage avant de sortir
                        remove_current_queue(ad->queue);
                        _comio2_free_queue_elem(e);
                        e=NULL;
                        
                        return_val=-1;
                        goto next_or_return;
                     }
                     
                     uint32_t tsp=_comio2_get_timestamp();
                     
                     if((tsp - e->tsp)<=10) // la reponse est pour nous et dans les temps (retour dans les 10 secondes)
                     {
                        // recuperation des donnees
                        memcpy(resp,&(e->frame[1]),e->l_frame-1); // on retire juste l'id
                        *l_resp=e->l_frame-1;
                        *comio2_err=e->comio2_err;
                        
                        // et on fait le menage avant de sortir
                        _comio2_free_queue_elem(e);
                        ad->queue->current->d=NULL; // pour evite le bug
                        remove_current_queue(ad->queue);
                        e=NULL;
                        
                        return_val=0;
                        goto next_or_return;
                     }
                     
                     // theoriquement pour nous mais donnees trop vieilles, on supprime ?
                     DEBUG_SECTION fprintf(stderr,"%s (%s) : data too old\n", DEBUG_STR,__func__);
                  }
                  else
                  {
                     DEBUG_SECTION fprintf(stderr,"%s (%s) : not for me (%d != %d)\n", DEBUG_STR,__func__, e->frame[1], frame_data_id);
                     e=NULL;
                  }
               }
            }
            while(next_queue(ad->queue)==0);
            notfound=1;
         }
      next_or_return:
         pthread_mutex_unlock(&(ad->sync_lock));
         pthread_cleanup_pop(0);
         
         if(return_val ==0 || return_val==-1)
         return return_val;
      }
      while (--boucle);
   }
error_exit:
   return -1;
}


int16_t comio2_setTrap(comio2_ad_t *ad, int16_t numTrap, trap_f trap, void *userdata)
{
   if(numTrap>0 && numTrap<=COMIO2_MAX_TRAP)
   {
      ad->tabTraps[numTrap-1].trap=trap;
      ad->tabTraps[numTrap-1].userdata=userdata;
      return 0;
   }
   return -1;
}


int16_t comio2_removeTrap(comio2_ad_t *ad, uint16_t numTrap)
{
   if(numTrap>0 && numTrap<=COMIO2_MAX_TRAP)
   {
      ad->tabTraps[numTrap-1].trap=NULL;
      ad->tabTraps[numTrap-1].userdata=NULL;
      return 0;
   }
   return -1;
}


void comio2_removeAllTraps(comio2_ad_t *ad)
{
   for(uint16_t i=1;i<=COMIO2_MAX_TRAP;i++)
   {
      ad->tabTraps[i-1].trap=NULL;
      ad->tabTraps[i-1].userdata=NULL;
   }
}


void _comio2_free_queue_elem(void *d) // pour vider_file2
{
   comio2_queue_elem_t *e=(comio2_queue_elem_t *)d;
   
   free(e);
   e=NULL;
}


int16_t _comio2_read_frame(int fd, char *cmd_data, uint16_t *l_cmd_data, int16_t *nerr)
{
   /*
    Start(1)[{] : octet de début de trame. Permet d'identifier le type de trame
    Longueur(1) : longueur de la réponse (start et end compris)
    Id_trame(1) : réponse à la question id_trame                    -+
    Response(1) : type de la réponse                                 | contenu dans cmd / l_cmd
    Data(x)     : données de la réponse                             -+
    CheckSum(1) : checksum calculé sur la trame hors start et end.
    End(1)[}]   : octet de fin de trame.
    */
   
   char c;
   fd_set input_set;
   struct timeval timeout;
   
   int16_t ret=0;
   uint16_t step=0;
   int16_t ntry=0;
   
   uint16_t i=0;
   uint16_t checksum=0;
   
   timeout.tv_sec  = 1; // timeout après 1 secondes
   timeout.tv_usec = 0;
   
   FD_ZERO(&input_set);
   FD_SET(fd, &input_set);
   
   *nerr=0;
   
   while(1)
   {
      ret = (int16_t)select(fd+1, &input_set, NULL, NULL, &timeout);
      if (ret <= 0)
      {
         if(ret == 0)
         *nerr=COMIO2_ERR_TIMEOUT;
         else
         *nerr=COMIO2_ERR_SELECT;
         goto on_error_exit_comio2_read;
      }
      
      ret=(int16_t)read(fd, &c, 1);
      if(ret!=1)
      {
         if(ntry>(COMIO2_NB_RETRY-1)) // 5 essais si pas de caratères lus
         {
            *nerr=COMIO2_ERR_READ;
            goto on_error_exit_comio2_read;
         }
         ntry++;
         continue; // attention, si aucun caractère lu on boucle
      }
      VERBOSE(9) fprintf(stderr, "%02x ",c & 0xFF);
      ntry=0;
      switch(step)
      {
         case 0:
            if(c=='{')
         {
            step++;
            break;
         }
            *nerr=COMIO2_ERR_STARTFRAME;
            goto on_error_exit_comio2_read;
            
         case 1:
            *l_cmd_data=c-4;
            checksum+=(unsigned char)c;
            i=0;
            step++;
            break;
            
         case 2:
            cmd_data[i]=c;
            checksum+=(unsigned char)c;
            i++; // maj du reste à lire
            if(i>=*l_cmd_data)
            step++; // read checksum
            break;
            
         case 3:
            if(((checksum+(unsigned char)c) & 0xFF) == 0xFF)
            {
              step++;
              break;
            }
            else
            {
               VERBOSE(5) fprintf(stderr,"%s  (%s) : Comio reponse - checksum error.\n",INFO_STR,__func__);
               *nerr=COMIO2_ERR_CHECKSUM;
               goto on_error_exit_comio2_read;
            }
            break;
            
         case 4:
            if(c=='}')
            {
               VERBOSE(9) fprintf(stderr, "\n");
              return 0;
            }
            *nerr=COMIO2_ERR_STOPFRAME;
            goto on_error_exit_comio2_read;
      }
   }
   *nerr=COMIO2_ERR_UNKNOWN; // ne devrait jamais se produire ...
   
on_error_exit_comio2_read:
   return -1;
}


int16_t _comio2_build_frame(char id, char *frame, char *cmd_data, uint16_t l_cmd_data)
{
   /*
    Start(1)[{] : octet de début de trame. Permet d'identifier le type de trame
    Longueur(1) : longueur de la réponse (start et end compris)
    Id_trame(1) : réponse à la question id_trame                    -+
    Response(1) : type de la réponse                                 | contenu dans cmd_data / l_cmd_data
    Data(x)     : données de la réponse                             -+
    CheckSum(1) : checksum calculé sur la trame hors Start et End.
    End(1)[}]   : octet de fin de trame.
    */
   
   uint16_t cchecksum = 0;
   
   frame[0]='[';
   
   cchecksum=4+l_cmd_data;
   frame[1]=4+l_cmd_data;
   
   cchecksum+=id;
   frame[2]=id;
   
   uint16_t i=0;
   for(;i<l_cmd_data;i++)
   {
      cchecksum+=cmd_data[i];
      frame[3+i]=cmd_data[i];
   }
   
   cchecksum=0xFF - (cchecksum & 0xFF);
   i+=3;
   frame[i++]=cchecksum;
   frame[i++]=']';
   
   return i;
}


int16_t _comio2_write_frame(int fd, char id, char *cmd_data, uint16_t l_cmd_data, int16_t *nerr)
{
   uint16_t l_frame=0;
   char *frame=NULL;
   int16_t ret;
   
   *nerr=0;
   
   frame=(char *)malloc(l_cmd_data+6);
   if(!frame)
   {
      *nerr=COMIO2_ERR_SYS;
      return -1;
   }
   
   l_frame=_comio2_build_frame(id,frame,cmd_data,l_cmd_data);
   
   ret=(int16_t)write(fd,frame,l_frame);

   free(frame);
   
   if(ret<0)
   {
      *nerr=COMIO2_ERR_SYS;
      return -1;
   }
   return 0;
}


void _comio2_flush_old_responses_queue(comio2_ad_t *ad)
{
   comio2_queue_elem_t *e;
   uint32_t tsp=_comio2_get_timestamp();
   
   pthread_cleanup_push( (void *)pthread_mutex_unlock, (void *)&(ad->sync_lock) );
   pthread_mutex_lock(&ad->sync_lock);
   
   if(first_queue(ad->queue)==0)
   {
      while(1)
      {
         if(current_queue(ad->queue, (void **)&e)==0)
         {
            if((tsp - e->tsp) > 10)
            {
               _comio2_free_queue_elem(e);
               remove_current_queue(ad->queue); // remove current passe sur le suivant
            }
            else
            next_queue(ad->queue);
         }
         else
         break;
      }
   }
   
   pthread_mutex_unlock(&ad->sync_lock);
   pthread_cleanup_pop(0);
}


int16_t _comio2_add_response_to_queue(comio2_ad_t *ad, char *frame, uint16_t l_frame)
{
   comio2_queue_elem_t *e;
   
   if(!ad)
   return -1;
   
   e=malloc(sizeof(comio2_queue_elem_t));
   if(e)
   {
      memcpy(e->frame,frame,l_frame);
      e->l_frame=l_frame;
      e->comio2_err=COMIO2_ERR_NOERR;
      e->tsp=_comio2_get_timestamp();
      
      pthread_cleanup_push( (void *)pthread_mutex_unlock, (void *)&(ad->sync_lock) );
      pthread_mutex_lock(&ad->sync_lock);
      
      in_queue_elem(ad->queue, e);
      
      if(ad->queue->nb_elem>=1)
      pthread_cond_broadcast(&ad->sync_cond);
      
      pthread_mutex_unlock(&ad->sync_lock);
      pthread_cleanup_pop(0);
   }
   
   return 0;
}


uint32_t _comio2_get_timestamp()
{
   return (uint32_t)time(NULL);
}


void *_comio2_thread(void *args)
{
   unsigned char frame[255];
   uint16_t l_frame;
   int16_t nerr;
   int16_t ret;
   
   comio2_ad_t *ad=(comio2_ad_t *)args;
   
   VERBOSE(5) fprintf(stderr,"%s  (%s) : starting comio2 read thread %s\n",INFO_STR,__func__,ad->serial_dev_name);
   while(1)
   {
      _comio2_flush_old_responses_queue(ad); // a chaque passage on fait le ménage
      
      ret=_comio2_read_frame(ad->fd, (char *)frame, &l_frame, &nerr);
      
      if(ret==0)
      {
         if(frame[0]>COMIO2_MAX_USER_FRAME_ID) // la requete est interne elle ne sera pas retransmise
         {
            switch(frame[0])
            {
               case COMIO2_TRAP_ID:
                  VERBOSE(9) printf("%s  (%s) : Trap #%d catched\n",INFO_STR,__func__,frame[1]);
                  if(ad->tabTraps[frame[1]-1].trap != NULL)
                  ad->tabTraps[frame[1]-1].trap(frame[1]-1, (char *)&(frame[2]), l_frame-2, ad->tabTraps[frame[1]-1].userdata);
                  VERBOSE(5) printf("%s  (%s) : no callback defined for trap %d\n",INFO_STR,__func__,frame[1]);
                  break;
            }
         }
         else
         {
            _comio2_add_response_to_queue(ad, (char *)frame, l_frame);
         }
      }
      if(ret<0)
      {
         switch(nerr)
         {
            case COMIO2_ERR_TIMEOUT:
               break;
            case COMIO2_ERR_SELECT:
            case COMIO2_ERR_READ:
            case COMIO2_ERR_SYS:
               VERBOSE(1) {
                  fprintf(stderr,"%s (%s) : communication error (nerr=%d).\n", ERROR_STR,__func__,nerr);
                  perror("");
               }
               ad->signal_flag=1;
               raise(SIGHUP);
               sleep(5); // on attend 5 secondes avant de s'arrêter seul.
               pthread_exit(NULL);

            default:
               VERBOSE(1) {
                  fprintf(stderr,"%s (%s) : error=%d.\n", ERROR_STR,__func__,nerr);
                  break;
               }
         }
      }
      pthread_testcancel();
   }
   pthread_exit(NULL);
}
