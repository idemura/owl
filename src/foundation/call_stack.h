#ifndef FOUNDATION_CALL_STACK_H
#define FOUNDATION_CALL_STACK_H

#include "foundation/lang.h"

#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

void call_stack_init(const char *bin_name);
void call_stack_dump(int fd, int sig);

#ifdef __cplusplus
}
#endif

#endif
