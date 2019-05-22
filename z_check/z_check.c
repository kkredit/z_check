/**
 * \file z_check.c
 *
 * \brief Implement z_check library logging functionality.
 * \details
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 * \license MIT
 */


/******************************************************************************
 *                                                                 Inclusions */
#include "z_check.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef Z_CHECK_HAS_SYSLOG
#include <syslog.h>
#endif


/******************************************************************************
 *                                                                    Defines */
#ifdef Z_CHECK_STATIC_CONFIG
    #define FUNC_MAYBE_UNUSED __attribute__((unused))
#else
    #define DEFAULT_MODULE_NAME ""
    #define FUNC_MAYBE_UNUSED
#endif

#define CONST_FUNC __attribute__((const))
#define MESSAGE_MAX_LEN 512
#define MAX_LEVEL_INDEX ((int)Z_DEBUG)

#ifdef Z_CHECK_HAS_SYSLOG
    #define ZL2SYSLOG_LEVEL(level) ((int)level)
#endif


/******************************************************************************
 *                                                                      Types */
typedef void (*ZLogFn_t)(const ZLogLevel_t level, const char const *file, const int line,
                         const char const *func, const char const *message);


/******************************************************************************
 *                                                      Function declarations */
static inline const char const * ZLog_LevelStr(const ZLogLevel_t level) CONST_FUNC;
static inline void ZLog_StdFile(FILE *outfile, const ZLogLevel_t level, const char const *file,
                                const int line, const char const *func, const char const *message);
static void ZLog_StdErr(const ZLogLevel_t level, const char const *file, const int line,
                        const char const *func, const char const *message) FUNC_MAYBE_UNUSED;
static void ZLog_StdOut(const ZLogLevel_t level, const char const *file, const int line,
                        const char const *func, const char const *message) FUNC_MAYBE_UNUSED;
#ifdef Z_CHECK_HAS_SYSLOG
static void ZLog_Syslog(const ZLogLevel_t level, const char const *file, const int line,
                        const char const *func, const char const *message) FUNC_MAYBE_UNUSED;
#endif


/******************************************************************************
 *                                                                       Data */
#ifndef Z_CHECK_STATIC_CONFIG
    /* Dynamically configured */
    static const char *m_moduleName = NULL;
    static ZLogFn_t m_ZLogFunc = NULL;
    static ZLogLevel_t m_logLevel;
    static ZLogLevel_t m_logLevelOrig;
#else
    /* Statically configured */
    #if !defined(Z_CHECK_MODULE_NAME) || !defined(Z_CHECK_LOG_FUNC) || !defined(Z_CHECK_INIT_LOG_LEVEL)
        #error "Must fully define Z_CHECK static configuration."
    #endif
    #if defined(Z_CHECK_HAS_SYSLOG)
        #error "Syslog version of Z_CHECK requires dynamic configuration"
    #endif

    static const char const *m_moduleName = Z_CHECK_MODULE_NAME;

    #if Z_CHECK_LOG_FUNC == Z_STDOUT
        static const ZLogFn_t m_ZLogFunc = ZLog_StdOut;
    #elif Z_CHECK_LOG_FUNC == Z_STDERR
        static const ZLogFn_t m_ZLogFunc = ZLog_StdErr;
    #else
        #error "invalid Z_CHECK_LOG_FUNC"
    #endif

    static ZLogLevel_t m_logLevel = Z_CHECK_INIT_LOG_LEVEL;
    static const ZLogLevel_t m_logLevelOrig = Z_CHECK_INIT_LOG_LEVEL;
#endif

static const char const *m_levelStrs[] = {
    "EMERGENCY",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG",
};


/******************************************************************************
 *                                                         External functions */
#ifndef Z_CHECK_STATIC_CONFIG
void ZLog_Open(const ZLogType_t logType, const ZLogLevel_t logLevel, const char const *moduleName) {
    if (NULL != m_ZLogFunc) {
        Z_LOG(Z_WARN, "called ZLog_Open() twice in same module, %s", m_moduleName);
    }
    else {
        m_moduleName = (NULL != moduleName) ? moduleName : DEFAULT_MODULE_NAME;
        m_logLevel = logLevel;
        m_logLevelOrig = logLevel;

        switch (logType) {
            case Z_STDERR:
                m_ZLogFunc = ZLog_StdErr;
                break;

            case Z_STDOUT:
                m_ZLogFunc = ZLog_StdOut;
                break;

#ifdef Z_CHECK_HAS_SYSLOG
            case Z_SYSLOG:
                openlog(m_moduleName, LOG_CONS, LOG_LOCAL0);
                m_ZLogFunc = ZLog_Syslog;
                break;
#endif

            default:
                /* don't have Z_LOG setup yet to use */
                fprintf(stderr, "Warning: Unknown log type (%d); falling back to stderr\n",
                        (int)logType);
                m_ZLogFunc = ZLog_StdErr;
                break;
        }
    }
}

void ZLog_Close(void) {
#ifdef Z_CHECK_HAS_SYSLOG
    if (ZLog_Syslog == m_ZLogFunc) {
        closelog();
    }
#endif

    m_ZLogFunc = NULL;
    m_moduleName = NULL;
}
#endif /* Z_CHECK_STATIC_CONFIG */

void ZLog_LevelSet(const ZLogLevel_t logLevel) {
    m_logLevel = logLevel;
}

void ZLog_LevelReset(void) {
    m_logLevel = m_logLevelOrig;
}

void ZLog(const ZLogLevel_t level, const char const *file, const int line, const char const *func,
          const char *format, ...) {
#ifndef Z_CHECK_STATIC_CONFIG
    if (NULL == m_ZLogFunc) {
        fprintf(stderr, "Error: May not use ZLog() before calling ZLog_Open()\n");
    }
    else
#endif

    ZD_RT_ASSERT((0 <= (int)level && MAX_LEVEL_INDEX >= (int)level),
                 "log level is an invalid value: %d", (int)level);

    if (m_logLevel >= level) {
        int rc;
        va_list args;
        char message[MESSAGE_MAX_LEN] = { 0 };

        va_start(args, format);
        rc = vsnprintf(message, MESSAGE_MAX_LEN, format, args);
        va_end(args);

        if (-1 == rc) {
            rc = snprintf(message, MESSAGE_MAX_LEN, "(failed to format message) %s", format);
        }

        if (0 < rc) {
            m_ZLogFunc(level, file, line, func, message);
        }
    }
}


/******************************************************************************
 *                                                         Internal functions */
static inline const char const * ZLog_LevelStr(const ZLogLevel_t level) {
    return m_levelStrs[(int)level];
}

static inline void ZLog_StdFile(FILE *outfile, const ZLogLevel_t level, const char const *file,
                                const int line, const char const *func, const char const *message) {
    fprintf(outfile, "%s: [%s] %s:%d:%s: %s\n",
            m_moduleName, ZLog_LevelStr(level), file, line, func, message);
}

static void ZLog_StdOut(const ZLogLevel_t level, const char const *file, const int line,
                        const char const *func, const char const *message) {
    ZLog_StdFile(stdout, level, file, line, func, message);
}

static void ZLog_StdErr(const ZLogLevel_t level, const char const *file, const int line,
                        const char const *func, const char const *message) {
    ZLog_StdFile(stderr, level, file, line, func, message);
}

#ifdef Z_CHECK_HAS_SYSLOG
static void ZLog_Syslog(const ZLogLevel_t level, const char const *file, const int line,
                        const char const *func, const char const *message) {
    syslog(ZL2SYSLOG_LEVEL(level), "[%s] %s:%d:%s: %s",
           ZLog_LevelStr(level), file, line, func, message);
}
#endif

