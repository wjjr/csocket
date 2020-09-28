/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _CSOCKET_LOG_H
#define _CSOCKET_LOG_H

#ifdef __C_DEBUG
#include <assert.h>
#elif !defined(_ASSERT_H)
#define assert(_) ((void) (0))
#endif

#undef EXIT_SUCCESS
#undef EXIT_FAILURE
#define EXIT_SUCCESS 0
#define EXIT_MISTAKE 1
#define EXIT_FAILURE 2
#define NOERR 0

#define BOOL_STR(x) ((x) ? "true" : "false")

#ifndef __MINGW32__
#define _PRINTF_FORMAT(f, f_params) __attribute__((format(printf, f, f_params)))
#else
#define _PRINTF_FORMAT(f, f_params) __attribute__((format(gnu_printf, f, f_params)))
#endif

enum log_level {
    FATAL,
    SILENT,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    NOISY
};

void log_increase_level(void);

void log_silence(void);

void _PRINTF_FORMAT(2, 3) log_print(enum log_level, const char *message_format, ...);

void _PRINTF_FORMAT(3, 4) __attribute__((noreturn)) die(unsigned char status, int err_num, const char *message_format, ...);

#ifndef __C_DEBUG
static __inline void _PRINTF_FORMAT(3, 4) log_debug(enum log_level log_level __attribute__((unused)), int err_num __attribute__((unused)), const char *message_format __attribute__((unused)), ...) {}
#else
void _PRINTF_FORMAT(3, 4) log_debug(enum log_level, int err_num, const char *message_format, ...);
#endif

#endif /* _CSOCKET_LOG_H */
