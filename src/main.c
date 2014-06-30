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
#include <errno.h>
#include <string.h>
#include <termios.h>
#include "debug.h"
#include "comio2.h"


int main(int argc, const char * argv[])
{
   comio2_ad_t *ad=NULL;
   int ret;
   char data[255];
   char resp[255];
   uint16_t l_resp=0;
   int16_t err;
   
   set_verbose_level(9);
   
   ad=(comio2_ad_t *)malloc(sizeof(comio2_ad_t));
   
   ret=comio2_init(ad, "/dev/tty.usbmodem1411", B9600);
   
   sleep(5);
   
   if(ret<0)
   {
      printf("Erreur - ");
      perror("\n");
      exit(1);
   }
   
    /*
    Start(1)[{] : octet de début de trame. Permet d'identifier le type de trame
    Longueur(1) : longueur de la réponse (start et end compris)
    Id_trame(1) : réponse à la question id_trame                    -+
    Response(1) : type de la réponse                                 | contenu dans cmd / l_cmd
    Data(x)     : données de la réponse                             -+
    CheckSum(1) : checksum calculé sur la trame hors start et end.
    End(1)[}]   : octet de fin de trame.
    */

   data[0]=1;
   data[1]=0;
   data[2]=2;
   data[3]=3;
   
   for(int i=0;i<30;i++)
   {
      printf("Boucle %d\n",i);
      ret = comio2_atCmdSendAndWaitResp(ad, 1, data, 4, resp, &l_resp, &err);
      printf("RET=%d : ",ret);
      if(!ret)
      {
         for(int i=1;i<l_resp;i++)
            printf("M[%d]=%02x ", data[i-1], resp[i] & 0xFF);
         printf("\n");
      }
      else
         printf("Error %d ...\n", err);
      sleep(3);
   }
   comio2_close(ad);
   
   return 0;
}
