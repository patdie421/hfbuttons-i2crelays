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

//
// Demandes xPL acceptées :
//
// schema_class = control
// schema_type = basic
//    type = relay|button
//    device = <adresse i2c du module/relai>
//    num = <numéro du relais>
//    action = s|r|t (si type = relay => t uniquement)
// 
// schema_class = sensor
// schema_class = request
//    request = current
//    type = relay
//    device = adresse i2c du module/relai
//    num = <numéro du relais>
//
void cmndMsgHandler(xPL_ServicePtr theService, xPL_MessagePtr theMessage, xPL_ObjectPtr userValue)
{
   xPL_NameValueListPtr ListNomsValeursPtr ;
   char *schema_type, *schema_class, *device, *type, *num;
   schema_class       = xPL_getSchemaClass(theMessage);
   schema_type        = xPL_getSchemaType(theMessage);
   ListNomsValeursPtr = xPL_getMessageBody(theMessage);
   device             = xPL_getNamedValue(ListNomsValeursPtr, device_str);
   type               = xPL_getNamedValue(ListNomsValeursPtr, type_str);
   num                = xPL_getNamedValue(ListNomsValeursPtr, num_str);
   
   
   VERBOSE(9) fprintf(stderr,"%s  (%s) : xPL Message to process : %s.%s\n",INFO_STR,__func__, schema_class, schema_type);

   if(strcmplower(schema_class, control_str) == 0 &&
      strcmplower(schema_type, basic_str) == 0)
   {
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
      if(!num)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no num\n",INFO_STR,__func__);
         return;
      }
      char *action = xPL_getNamedValue(ListNomsValeursPtr, num_str);
      if(!action)
      {
         VERBOSE(5) fprintf(stderr,"%s  (%s) : xPL message no action\n",INFO_STR,__func__);
         return;
      }
      
      // traiter ici la demande
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


