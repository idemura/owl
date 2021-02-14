#include "foundation/call_stack.h"

#include <execinfo.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_LEN 120

static const char *s_bin_name;

static void fd_printf(int fd, const char *fmt, ...)
{
    static char buf[LINE_LEN + 8];

    buf[LINE_LEN] = '.';
    buf[LINE_LEN + 1] = '.';
    buf[LINE_LEN + 2] = '.';
    buf[LINE_LEN + 3] = 0;

    va_list va;
    va_start(va, fmt);
    vsnprintf(buf, LINE_LEN, fmt, va);
    va_end(va);

    write(fd, buf, strlen(buf));
}

static void sig_segv_handler(int sig)
{
    fflush(stdout);
    fflush(stderr);

    call_stack_dump(STDERR_FILENO, sig);

    exit(1);
}

void call_stack_init(const char *bin_name)
{
    s_bin_name = bin_name;
    signal(SIGSEGV, &sig_segv_handler);
}

void call_stack_dump(int fd, int sig)
{
    fd_printf(fd, "binary: %s\n", s_bin_name);
    fd_printf(fd, "signal: %d\n", sig);
    void *st[25];
    int st_size = backtrace(st, array_sizeof(st));
    fd_printf(fd, "stack_trace:\n");
    for (int i = 0; i < st_size; i++) {
        fd_printf(fd, "  0x%p\n", st[i]);
    }
    fd_printf(fd, "\n");
}
