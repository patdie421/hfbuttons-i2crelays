//
//  error.h
//
//  Created by Patrice Dietsch on 17/05/13.
//
//

#ifndef mea_eDomus_error_h
#define mea_eDomus_error_h

typedef enum error_e
{
   ERROR=-1,
   NOERROR=0
} mea_error_t;

extern char *_error_str;
extern char *_info_str;
extern char *_debug_str;
extern char *_malloc_error_str;
extern char *_fatal_error_str;
extern char *_warning_str;

#define ERROR_STR _error_str
#define INFO_STR _info_str
#define DEBUG_STR _debug_str
#define MALLOC_ERROR_STR _malloc_error_str
#define FATAL_ERROR_STR _fatal_error_str
#define WARNING_STR _warning_str
#endif
