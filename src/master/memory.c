//
//  memory.c
//
//  Created by Patrice DIETSCH on 17/09/12.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "error.h"
#include "debug.h"


char *string_malloc_and_copy(char *str, int v)
{
   char *new_str=(char *)malloc(strlen(str)+1);
   if(new_str)
      strcpy(new_str,str);
   else
   {
      VERBOSE(1) {
         fprintf (stderr, "%s (%s) : %s - ",ERROR_STR,__func__,MALLOC_ERROR_STR);
         perror("");
      }
      return NULL;
   }
   return new_str;
}


char *string_free_malloc_and_copy(char **org_str, char *str, int v)
{
   if(*org_str)
   {
      free(*org_str);
      *org_str=NULL;
   }
   
   *org_str=(char *)malloc(strlen(str)+1);
   if(*org_str)
      strcpy(*org_str,str);
   else
   {
      VERBOSE(1) {
         fprintf (stderr, "%s (%s) : %s - ",ERROR_STR,__func__,MALLOC_ERROR_STR);
         perror("");
      }
      return NULL;
   }
   return *org_str;
}


