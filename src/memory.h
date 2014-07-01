//
//  memory.h
//
//  Created by Patrice DIETSCH on 17/09/12.
//
//

#ifndef __memory_h
#define __memory_h

#define FREE(x) if((x)) { free(x); x=NULL; }

//void display_malloc_error(char *f, char *file, int line);
char *string_free_malloc_and_copy(char **org_str, char *str,int v);
//char *string_malloc_and_copy(char *str,int v);
char *string_malloc_and_copy(char *str,int v);
#endif

