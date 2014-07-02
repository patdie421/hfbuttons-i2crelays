//
//  main.c
//  Comio2
//
//  Created by Patrice Dietsch on 23/06/2014.
//  Copyright (c) 2014 Patrice Dietsch. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
//#include <errno.h>
//#include <string.h>
#include <termios.h>

#include "debug.h"
#include "comio2.h"
#include "xplServer.h"

/*
  Format d'une trame "question" COMIO2
  Start(1)[{] : octet de début de trame. Permet d'identifier le type de trame
  Longueur(1) : longueur de la réponse (start et end compris)
  Id_trame(1) : réponse à la question id_trame
  Response(1) : type de la réponse
  Data(x)     : données de la réponse
  CheckSum(1) : checksum calculé sur la trame hors start et end.
  End(1)[}]   : octet de fin de trame.
*/


int16_t set_xpl_address()
/**
 * \brief     initialise les données pour l'adresse xPL
 * \details   positionne vendorID, deviceID et instanceID pour xPLServer
 * \param     params_liste  liste des parametres.
 * \return   -1 en cas d'erreur, 0 sinon
 */
{
   set_xPL_vendorID("mea");
   set_xPL_deviceID("hfedomus");
   set_xPL_instanceID("inst1");
   
   return 0;
}


int16_t test_trap(int16_t num, char *data, int16_t l_data, void *userdata)
{
   printf("TRAP #%d : %s\n", num, (char *)userdata);
   printf("Data : ");
   for(int i=0;i<l_data;i++)
      printf("D[%d]=%02x(%03u) ", i, data[i] & 0xFF, data[i] & 0xFF);
   printf("\n");

   return 0;
}


void test1(comio2_ad_t *ad)
{
   char data[255];
   char resp[255];
   uint16_t l_resp=0;
   int ret;
   int16_t err;

   printf("Test accès mémoire en lecture/écriture\n");
   printf("--------------------------------------\n");
   for(int i=0;i<30;i++)
   {
      printf("Boucle %d :\n",i);
      
      data[0]=1; // lecture adresse 1
      data[1]=0; // lecture adresse 0
      ret = comio2_atCmdSendAndWaitResp(ad, COMIO2_CMD_READMEMORY, data, 2, resp, &l_resp, &err);
      printf("RETCODE : %d\n",ret);
      if(!ret)
      {
         for(int i=1;i<l_resp;i++)
            printf("M[%d]=%02x(%03u) ", data[i-1], resp[i] & 0xFF, resp[i] & 0xFF);
         printf("\n");
      }
      else
         printf("ERRORCODE = %d\n", err);
      
      data[0]=1; // écriture adresse mémoire 1
      data[1]=i; // avec valeur de i
      comio2_atCmdSend(ad, COMIO2_CMD_WRITEMEMORY, data, 2, &err);
   }
}


void test2(comio2_ad_t *ad)
{
   char data[255];
   char resp[255];
   uint16_t l_resp=0;
   int ret;
   int16_t err;

   data[0]=2; // numéro de la fonction
   data[1]=6; // paramètre de la fonction
   data[2]=2; // paramètre de la fonction
   
   printf("Test appel de fonctions\n");
   printf("-----------------------\n");
   for(int i=0;i<30;i++)
   {
      printf("Boucle %d :\n",i);
      ret = comio2_atCmdSendAndWaitResp(ad, COMIO2_CMD_CALLFUNCTION, data, 3, resp, &l_resp, &err);
      printf("RETCODE : %d\n", ret);
      if(!ret)
      {
         if(resp[0] == COMIO2_CMD_CALLFUNCTION)
         {
            for(int i=1;i<l_resp;i++)
            {
               printf("R[%d]=%02x(%03u) ", i-1, resp[i] & 0xFF, resp[i] & 0xFF);
            }
            printf("\n");
         }
         else
            printf("Erreur ... %d\n", resp[0]);
      }
      else
         printf("ERRORCODE = %d\n", err);
   }
}


static char *serial = "/dev/ttyATH0";

int main(int argc, const char * argv[])
{
   comio2_ad_t *ad=NULL;
   int ret;
   char *userdata1="test user data1";
   char *userdata2="test user data2";
   
   set_verbose_level(9);

   set_xpl_address();
   
   xPLServer();
   
   ad=comio2_new_ad();
   ret=comio2_init(ad, serial, B115200);
   if(ret<0)
   {
      fprintf(stderr, "ERROR - %s ", serial);
      perror("");
      fprintf(stderr, "\n");
      exit(1);
   }
   comio2_setTrap(ad, 1, test_trap, userdata1);
   comio2_setTrap(ad, 2, test_trap, userdata2);
   
   sleep(2); // pour connexion USB, attendre la fin du reboot du mcu
   
   test1(ad);
   printf("\n");
   
   test2(ad);
   printf("\n");
   
   comio2_close(ad);
      
   return 0;
}

