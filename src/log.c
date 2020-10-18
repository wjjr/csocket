/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static enum log_level log_level = WARN;

static __inline char error_prefix(const enum log_level log_lvl) {
    switch (log_lvl) {
        case FATAL:
        case ERROR:
            return 'E';
        case WARN:
            return 'W';
        case INFO:
            return 'I';
        case DEBUG:
            return 'D';
        default:
            return '*';
    }
}

static void error_log(const enum log_level log_lvl, const unsigned char status, const int err_num, const char *message_format, va_list args) {
    fprintf(stderr, "%c: ", error_prefix(log_lvl));

    vfprintf(stderr, message_format, args);

    if (err_num)
        fprintf(stderr, ": %s", strerror(err_num));

    putc('\n', stderr);

    if (status)
        exit(status);
    else if (log_lvl == FATAL)
        exit(EXIT_FAILURE);
}

void log_increase_level(void) {
    if (log_level != SILENT)
        ++log_level;
}

void log_silence(void) {
    log_level = SILENT;
}

void log_print(const enum log_level log_lvl, const char *const message_format, ...) {
    if (log_lvl <= log_level) {
        va_list args;

        va_start(args, message_format);
        error_log(log_lvl, 0, 0, message_format, args);
        va_end(args);
    }
}

void die(const unsigned char status, const int err_num, const char *const message_format, ...) {
    va_list args;

    va_start(args, message_format);
    error_log(FATAL, status, err_num, message_format, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

#ifdef __C_DEBUG
void log_debug(enum log_level log_lvl, int err_num, const char *const message_format, ...) {
    if (log_lvl <= log_level) {
        va_list args;

        va_start(args, message_format);
        error_log(log_lvl, 0, err_num, message_format, args);
        va_end(args);
    }
}
#endif
