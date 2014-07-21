/*
 *  debug.h
 *
 *  Created by Patrice Dietsch on 14/08/12.
 *  Copyright 2012 -. All rights reserved.
 *
 */

#ifndef __debug_h
#define __debug_h

#include <inttypes.h>

#define DEBUG_SECTION if(debug_msg)
#define VERBOSE(v) if(v <= verbose_level)

extern int debug_msg;
extern int verbose_level;

void debug_on();
void debug_off();
int16_t debug_status();
void set_verbose_level(int level);

uint32_t start_chrono(uint32_t *_last_time);
uint32_t take_chrono(uint32_t *_last_time);

#endif


