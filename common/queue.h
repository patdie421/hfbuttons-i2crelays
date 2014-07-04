/*
 *  queue.h
 *
 *  Created by Patrice Dietsch on 28/07/12.
 *  Copyright 2012 -. All rights reserved.
 *
 */

#ifndef __queue_h
#define __queue_h

#include "error.h"

typedef void (*free_data_f)(void *);

struct queue_elem
{
   void *d;
   struct queue_elem *next;
   struct queue_elem *prev;
};

typedef struct
{
   struct queue_elem *first;
   struct queue_elem *last;
   struct queue_elem *current;
   unsigned long nb_elem;
} queue_t;


mea_error_t init_queue(queue_t *queue);
mea_error_t out_queue_elem(queue_t *queue, void **data);
mea_error_t in_queue_elem(queue_t *queue, void *data);
mea_error_t process_all_elem_queue(queue_t *queue, void (*f)(void *));
mea_error_t first_queue(queue_t *queue);
mea_error_t last_queue(queue_t *queue);
mea_error_t next_queue(queue_t *queue);
mea_error_t prev_queue(queue_t *queue);
mea_error_t current_queue(queue_t *queue, void **data);
mea_error_t clear_queue(queue_t *queue,free_data_f f);
mea_error_t remove_current_queue(queue_t *queue);

#endif
