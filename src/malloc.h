#ifndef MALLOC_H
#define MALLOC_H

#include "setup.h"

#ifdef STDC_HEADERS
# include <stdlib.h>
#endif

void *MyMalloc(size_t bytes);
void MyFree(void *var);

#endif /* MALLOC_H */
