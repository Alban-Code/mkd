#ifndef LOG_H
#define LOG_H

typedef enum
{
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} log_level_t;

void log_init(log_level_t level);

void log_write(log_level_t level, const char *file, int line, const char *fmt, ...);

#define LOG(level, ...) \
    log_write(level, __FILE__, __LINE__, __VA_ARGS__)

#endif