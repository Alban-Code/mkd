#include "kernel/log.h"
#include <stdarg.h>
#include <stdio.h>

static log_level_t g_level = LOG_INFO;

static const char *level_names[] = {"ERROR", "WARN", "INFO", "DEBUG"};

void log_init(log_level_t level)
{
    g_level = level;
}

void log_write(log_level_t level, const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (level <= g_level)
    {
        FILE *out = (level == LOG_ERROR) ? stderr : stdout;
        fprintf(out, "[%s] %s:%d - ", level_names[level], file, line);
        vfprintf(out, fmt, args);
        fprintf(out, "\n");
    }
    va_end(args);
}