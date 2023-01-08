#ifndef __P_UTILS_H__
#define __P_UTILS_H__ 1

#define OOM_MESSAGE
#include <stdio.h>
#include <stdlib.h>

void check_not_failed_or_exit(const void const *ptr,
                              const char const *allocation_name) {
  if (ptr == NULL) {
    printf("Got an out of memory error allocating %s. Exiting\n",
           allocation_name);
    exit(1);
  }
}

#endif