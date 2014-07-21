/*
 *  queue.c
 *
 *  Created by Patrice Dietsch on 28/07/12.
 *  Copyright 2012 -. All rights reserved.
 *
 */
#include "queue.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>


unsigned long nb_queue_elem(queue_t *queue)
{
   if(!queue)
      return -1;

   return queue->nb_elem;
}


mea_error_t in_queue_elem(queue_t *queue, void *data)
{
   struct queue_elem *new;
   
   if(!queue)
      return ERROR;

   new=(struct queue_elem *)malloc(sizeof(struct queue_elem));
   if(!new)
      return ERROR;
   
   new->d=data;
   
   if(queue->first==NULL)
   {
      queue->first=new;
      queue->last=new;
      new->next=NULL;
      new->prev=NULL;
   }
   else
   {
      new->next=queue->first;
      new->prev=NULL;
      queue->first->prev=new;
      queue->first=new;
   }
   
   queue->nb_elem++;
   
   return NOERROR;
}


mea_error_t out_queue_elem(queue_t *queue, void **data)
{
   struct queue_elem *ptr;
   
   if(!queue)
      return ERROR;
   
   if(queue->last)
   {
      ptr=queue->last;
      
      if(queue->last == queue->first)
      {
         queue->last=NULL;
         queue->first=NULL;
      }
      else
      {
         queue->last=queue->last->prev;
         queue->last->next=NULL;
      }
      *data=ptr->d;
      free(ptr);
      ptr=NULL;
   }
   else 
      return ERROR;
   
   queue->nb_elem--;
   
   return NOERROR;
}


mea_error_t init_queue(queue_t *queue)
{
   if(!queue)
      return ERROR;

   queue->first=NULL;
   queue->last=NULL;
   queue->current=NULL;
   queue->nb_elem=0;
   
   return NOERROR;
}


mea_error_t first_queue(queue_t *queue)
{
    if(!queue || !queue->first)
      return ERROR;
   
   queue->current=queue->first;
   return NOERROR;
}


mea_error_t last_queue(queue_t *queue)
{
   if(!queue || !queue->last)
      return ERROR;
   
   queue->current=queue->last;
   return NOERROR;
}


mea_error_t next_queue(queue_t *queue)
{
   if(!queue || !queue->current)
      return ERROR;
   
   if(!queue->current->next)
   {
      queue->current=NULL;
      return ERROR;
   }
   
   queue->current=queue->current->next;
   return NOERROR;
}


mea_error_t prev_queue(queue_t *queue)
{
   if(!queue || !queue->current)
      return ERROR;
   if(!queue->current->prev)
   {
      queue->current=NULL;
      return ERROR;
   }
   
   queue->current=queue->current->prev;
   
   return NOERROR;
}


mea_error_t clear_queue(queue_t *queue,free_data_f f)
{
   struct queue_elem *ptr;
   
   if(!queue)
      return ERROR;

   while(queue->nb_elem>0)
   {
      if(queue->last)
      {
         ptr=queue->last;
         
         if(queue->last == queue->first)
         {
            queue->last=NULL;
            queue->first=NULL;
         }
         else
         {
            queue->last=queue->last->prev;
            queue->last->next=NULL;
         }
         if(ptr->d)
         {
            if(f)
               f(ptr->d);
         }
         free(ptr);
         ptr=NULL;
      }
      else 
         return NOERROR;
      
      queue->nb_elem--;		
   }
   return NOERROR;
}


mea_error_t current_queue(queue_t *queue, void **data)
{
   if(!queue)
      return ERROR;

   if(!queue->current)
      return ERROR;
   
   *data=queue->current->d;
   
   return NOERROR;
}


mea_error_t remove_current_queue(queue_t *queue)
{
   if(!queue)
      return ERROR;

    if(queue->nb_elem==0)
       return ERROR;
   
    if(queue->nb_elem==1)
    {
       free(queue->current);
       queue->current=NULL;
       queue->first=NULL;
       queue->last=NULL;
       queue->nb_elem=0;
       return NOERROR;
    }
   
   struct queue_elem *prev;
   struct queue_elem *next;
   struct queue_elem *old;

   prev=queue->current->prev;
   next=queue->current->next;
   old=queue->current;
   
   if(prev)
   {
      prev->next=queue->current->next;
   }
   
   if(next)
   {
      next->prev=queue->current->prev;
   }
   
   if(queue->current==queue->first)
      queue->first=queue->current->next;
   
   if(queue->current==queue->last)
      queue->last=queue->current->prev;
   
   queue->nb_elem--;
   
   queue->current=next;
   
   free(old);
   
   return NOERROR;
}


mea_error_t process_all_queue_elem(queue_t *queue, void (*f)(void *))
{
   struct queue_elem *ptr;
   
   if(!queue)
      return ERROR;

   ptr=queue->first;
   if(!ptr)
      return ERROR;
   do
   {
      f(ptr->d);
      ptr=ptr->next;
      
   } while (ptr);
   
   return NOERROR;
}

