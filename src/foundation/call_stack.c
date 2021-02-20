#include "foundation/call_stack.h"

#include <libunwind.h>
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

    fd_printf(fd, "stack:\n");

    unw_context_t ctx;
    unw_getcontext(&ctx);

    unw_cursor_t cursor;
    unw_init_local(&cursor, &ctx);

    const int max_frames = 40;
    int c_frame = 0;
    while (unw_step(&cursor) > 0 && c_frame < max_frames) {
        unw_word_t ip, sp, offset = 0;
        char buf[80];

        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);

        if (unw_get_proc_name(&cursor, buf, sizeof buf, &offset) == 0) {
            if (c_frame == 0
                    && (str_starts_with(buf, "_sig") || strcmp(buf, "sig_segv_handler") == 0)) {
                continue;
            }
        } else {
            buf[0] = 0;
        }

        fd_printf(fd, "  ip=0x%lx sp=0x%lx", (long) ip, (long) sp);
        if (buf[0]) {
            fd_printf(fd, " %s +0x%02lx", buf, (long) offset);
        }
        fd_printf(fd, "\n");

        c_frame++;
    }

    if (c_frame == max_frames) {
        fd_printf(fd, "  ...\n");
    }

    fd_printf(fd, "\n");
}
