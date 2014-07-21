//
//  xPLServer.c
//
//  Created by Patrice DIETSCH on 17/10/12.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include "xPL.h"
#include "debug.h"
#include "error.h"
#include "memory.h"

#define XPL_VERSION "0.1a2"

xPL_ServicePtr xPLService = NULL;


char *xpl_vendorID=NULL;
char *xpl_deviceID=NULL;
char *xpl_instanceID=NULL;


int16_t strcmplower(char *str1, char *str2)
/**
 * \brief     comparaison de deux chaines sur la base de "caractères en mimuscules"
 * \details   chaque caractère des deux chaines est converti en minuscule avant de les comparer
 * \param     str1   premiere chaine à comparer.
 * \param     str2   deuxième chaine à comparer.
 * \return    0 chaines égales, 1 chaines différentes
 */
{
   int i;
   for(i=0;str1[i];i++) {
      if(tolower(str1[i])!=tolower(str2[i]))
         return 1;
   }
   if(str1[i]!=str2[i])
      return 1;
   return 0;
}


int16_t isnumber(char *str)
{
   for(int i=0;i<strlen(str);i++)
     if(str[i]<'0' || str[i]>'9')
       return 0;
   return -1;
}


char *set_xPL_vendorID(char *value)
{
   return string_free_malloc_and_copy(&xpl_vendorID, value, 1);
}


char *set_xPL_deviceID(char *value)
{
   return string_free_malloc_and_copy(&xpl_deviceID, value, 1);
}


char *set_xPL_instanceID(char *value)
{
   return string_free_malloc_and_copy(&xpl_instanceID, value, 1);
}


xPL_ServicePtr get_xPL_ServicePtr()
{
   return xPLService;
}

static char *request_str="request";
static char *control_str="control";
static char *basic_str="basic";
static char *device_str="device";
static char *type_str="type";
static char *current_str="current";
static char *num_str="num";
static char *sensor_str="sensor";
static char *action_str="action";
static char *relay_str="relay";
static char *button_str="button";
static char *addr_str="addr";


void xplSend(char *device, char *addr, char *num, char *stat)
{
   xPL_ServicePtr servicePtr = get_xPL_ServicePtr();
   if(servicePtr)
   {
      xPL_MessagePtr cntrMessageStat = xPL_createBroadcastMessage(servicePtr, xPL_MESSAGE_TRIGGER);
     
      xPL_setSchema(cntrMessageStat, sensor_str, basic_str);
      xPL_setMessageNamedValue(cntrMessageStat, device_str,device);
      xPL_setMessageNamedValue(cntrMessageStat, type_str, relay_str);
      xPL_setMessageNamedValue(cntrMessageStat, current_str,stat);
   
      // Broadcast the message
      xPL_sendMessage(cntrMessageStat);
     
      xPL_releaseMessage(cntrMessageStat);
   }
}


//
// Demandes xPL acceptées :
//
// schema_class = control
// schema_type = basic
//    device = <nom>
//    type = relay|button
//    addr = <adresse i2c du module/relai>
//    num = <numéro du relais>
//    action = s|r|t|p (si type = button, p uniquement, pour les relais s,r et t uniquement)
// 
// schema_class = sensor
// schema_class = request
//    request = current
//    device = <nom>
//    type = relay
//    addr = <adresse i2c du module/relai>
//    num = <numéro du relais>
//    num = <numéro du relais>
//
void cmndMsgHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
   xPL_NameValueListPtr ListNomsValeursPtr ;
   char *schema_type, *schema_class, *device, *type, *addr, *num;
   schema_class       = xPL_getSchemaClass(theMessage);
   schema_type        = xPL_getSchemaType(theMessage);
   ListNomsValeursPtr = xPL_getMessageBody(theMessage);
   device             = xPL_getNamedValue(ListNomsValeursPtr, device_str);
   type               = xPL_getNamedValue(ListNomsValeursPtr, type_str);
   num                = xPL_getNamedValue(ListNomsValeursPtr, num_str);
   addr               = xPL_getNamedValue(ListNomsValeursPtr, addr_str);
   
   
   VERBOSE(9) fprintf(stderr,"%s  (%s) : xPL Message to process : %s.%s\n",INFO_STR,__func__, schema_class, schema_type);
   int error=0;
   if(strcmplower(schema_class, control_str) == 0 &&
      strcmplower(schema_type, basic_str) == 0)
   {
      if(!type)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no type\n",INFO_STR,__func__);
         error++;
      }
      if(!device)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no device\n",INFO_STR,__func__);
         error++;
      }
      if(!addr)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no addr\n",INFO_STR,__func__);
         error++;
      }
      if(!num)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no num\n",INFO_STR,__func__);
         error++;
      }
      
      char *action = xPL_getNamedValue(ListNomsValeursPtr, action_str);
      if(!action)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no action\n",INFO_STR,__func__);
         error++;
      }
        
      if(!isnumber(addr) || !isnumber(num))
      {
        VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message numeric error (addr or num)\n",INFO_STR,__func__);
        error++;
      }
      
      if(action[1]!=0)
      {
        VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message action error (must be :'s', 'r' or 't')\n",INFO_STR,__func__);
        error++;
      }
      action[0]=tolower(action[0]);
      
      // traiter ici la demande
      if(strcmplower(type,relay_str)==0)
      {
        int iaddr=atoi(addr);
        int inum=atoi(num);
        if(iaddr>127 || inum>2)
        {
          VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message value error (addr or num)\n",INFO_STR,__func__);
          error++;
        }

        switch(action[0])
        {
           case 's':
           case 'r':
           case 't':
             if(!error)
                printf("OK relais pour %s : %d %d %c\n",device,iaddr,inum,action[0]);
             else
             {
                VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message can't process\n",INFO_STR,__func__);
             }
             break;
           default:
             VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message action error\n",INFO_STR,__func__);
             error++;
             break;
        }
      }
      else if(strcmplower(type,button_str)==0)
      {
        int iaddr=atoi(addr);
        int inum=atoi(num);
        if(iaddr>63 || inum>2)
        {
          VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message value error (addr or num)\n",INFO_STR,__func__);
          error++;
        }

        switch(action[0])
        {
          case 'p':
            if(!error)
              printf("OK bouton pour %s : %d %d %c\n",device,iaddr,inum,action[0]);
            else
            {
                VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message can't process\n",INFO_STR,__func__);
            }
            break;
          default:
            VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message action error (must be :'p')\n",INFO_STR,__func__);
            error++;
        }
     }
     else
     {
        VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message type error\n",INFO_STR,__func__);
     }
     return;
   }
   else if(strcmplower(schema_class,"sensor") == 0 &&
           strcmplower(schema_type, "request") == 0)
   {
      char *request = xPL_getNamedValue(ListNomsValeursPtr, request_str);
      if(!request)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no request\n",INFO_STR,__func__);
         return;
      }
      if(strcmplower(request,current_str)!=0)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message request!=current\n",INFO_STR,__func__);
         return;
      }
      if(!type)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no type\n",INFO_STR,__func__);
         return;
      }
      if(!device)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no device\n",INFO_STR,__func__);
         return;
      }
      if(!addr)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no addr\n",INFO_STR,__func__);
         return;
      }
      if(!num)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no num\n",INFO_STR,__func__);
         return;
      }
     
      // traiter ici la demande
      return;
   }
}


void *_xPL_server_thread(void *data)
{
   VERBOSE(9) fprintf(stderr,"%s  (%s) : xPLServer thred started\n",INFO_STR,__func__);

   if ( !xPL_initialize(xPL_getParsedConnectionType()) ) return 0 ;
   
   xPLService = xPL_createService(xpl_vendorID, xpl_deviceID, xpl_instanceID);
   xPL_setServiceVersion(xPLService, XPL_VERSION);
   
   xPL_setRespondingToBroadcasts(xPLService, TRUE);
   
   // xPL_setHeartbeatInterval(xPLService, 5000); // en milliseconde ?
   // xPL_MESSAGE_ANY, xPL_MESSAGE_COMMAND, xPL_MESSAGE_STATUS, xPL_MESSAGE_TRIGGER
   
   xPL_addServiceListener(xPLService, cmndMsgHandler, xPL_MESSAGE_COMMAND, control_str, basic_str, (xPL_ObjectPtr)data) ;
   xPL_addServiceListener(xPLService, cmndMsgHandler, xPL_MESSAGE_COMMAND, sensor_str, request_str, (xPL_ObjectPtr)data) ;
   
   xPL_setServiceEnabled(xPLService, TRUE); 

   do
   {
      VERBOSE(9) {
         static char compteur=0;
         if(compteur>59)
         {
            compteur=0;
            fprintf(stderr,"%s  (%s) : xPLServer thread actif\n",INFO_STR,__func__);
         }
         else
            compteur++;
      }
     
      xPL_processMessages(500);
      pthread_testcancel();
   }
   while (1);
}


pthread_t *xPLServer()
{
   pthread_t *xPL_thread=NULL;

   if(!xpl_deviceID || !xpl_instanceID || !xpl_vendorID)
   {
      VERBOSE(1) fprintf(stderr, "%s (%s) : xPL address not set\n",ERROR_STR,__func__);
      return NULL;
   }

   xPL_thread=(pthread_t *)malloc(sizeof(pthread_t));
   if(!xPL_thread)
   {
      VERBOSE(1) {
         fprintf (stderr, "%s (%s) : malloc - ",ERROR_STR,__func__);
         perror("");
      }
      return NULL;
   }

   if(pthread_create (xPL_thread, NULL, _xPL_server_thread, NULL))
   {
      VERBOSE(1) fprintf(stderr, "%s (%s) : pthread_create - can't start thread\n",ERROR_STR,__func__);
      return NULL;
   }
     
   return xPL_thread;
}


