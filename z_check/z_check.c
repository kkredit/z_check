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
#define DEFAULT_MODULE_NAME "Unnamed Module"
#define MESSAGE_MAX_LEN 512
#define ZL_STR(level) \
    (Z_EMERG  == (level) ? "EMERGENCY" : \
     (Z_ALERT  == (level) ? "ALERT" : \
      (Z_CRIT   == (level) ? "CRITICAL" : \
       (Z_ERR    == (level) ? "ERROR" : \
        (Z_WARN   == (level) ? "WARNING" : \
         (Z_NOTICE == (level) ? "NOTICE" : \
          (Z_INFO   == (level) ? "INFO" : \
           (Z_DEBUG  == (level) ? "DEBUG" : "UNKNOWN_LEVEL"))))))))
#define ZL2SYSLOG_LEVEL(level) ((int)level)


/******************************************************************************
 *                                                                      Types */
typedef void (*ZLogFn_t)(ZLogLevel_t level, const char *file, int line, const char *func);


/******************************************************************************
 *                                                                       Data */
#ifndef Z_CHECK_STATIC_CONFIG

/* Dynamically configured */
static const char *m_moduleName = NULL;
static ZLogFn_t m_ZLogFunc = NULL;
static ZLogLevel_t m_logLevel;
static ZLogLevel_t m_logLevelOrig;

#define FUNC_USED

#else /* Z_CHECK_STATIC_CONFIG */

/* Statically configured */
#if !defined(Z_CHECK_MODULE_NAME) || !defined(Z_CHECK_LOG_FUNC) || !defined(Z_CHECK_INIT_LOG_LEVEL)
    #error "Must fully define Z_CHECK static configuration."
#endif
#if defined(Z_CHECK_HAS_SYSLOG)
    #error "Syslog version of Z_CHECK requires dynamic configuration"
#endif

#define FUNC_USED  __attribute__((unused))

static const char *m_moduleName = Z_CHECK_MODULE_NAME;
#if Z_CHECK_LOG_FUNC == Z_STDOUT
    #define m_ZLogFunc ZLog_StdOut
#elif Z_CHECK_LOG_FUNC == Z_STDERR
    #define m_ZLogFunc ZLog_StdErr
#else
    #error "invalid Z_CHECK_LOG_FUNC"
#endif
static ZLogLevel_t m_logLevel = Z_CHECK_INIT_LOG_LEVEL;
static ZLogLevel_t m_logLevelOrig = Z_CHECK_INIT_LOG_LEVEL;

#endif /* Z_CHECK_STATIC_CONFIG */

static char m_message[MESSAGE_MAX_LEN] = { 0 };


/******************************************************************************
 *                                                      Function declarations */
static inline void ZLog_StdFile(FILE *outfile, ZLogLevel_t level, const char *file, int line,
                                const char *func);
static void ZLog_StdErr(ZLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
static void ZLog_StdOut(ZLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
#ifdef Z_CHECK_HAS_SYSLOG
static void ZLog_Syslog(ZLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
#endif


/******************************************************************************
 *                                                         External functions */
#ifndef Z_CHECK_STATIC_CONFIG
void ZLog_Open(ZLogType_t logType, ZLogLevel_t logLevel, const char *moduleName) {
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
    memset(m_message, 0, sizeof(m_message));
}
#endif /* Z_CHECK_STATIC_CONFIG */

void ZLog_LevelSet(ZLogLevel_t logLevel) {
    m_logLevel = logLevel;
}

void ZLog_LevelReset(void) {
    m_logLevel = m_logLevelOrig;
}

void ZLog(ZLogLevel_t level, const char *file, int line, const char *func,
          const char *format, ...) {
#ifndef Z_CHECK_STATIC_CONFIG
    if (NULL == m_ZLogFunc) {
        fprintf(stderr, "Error: May not use ZLog() before calling ZLog_Open()\n");
    }
    else
#endif /* Z_CHECK_STATIC_CONFIG */
    if (m_logLevel >= level) {
        int rc;
        va_list args;
        va_start(args, format);
        rc = vsnprintf(m_message, MESSAGE_MAX_LEN, format, args);
        va_end(args);

        if (-1 == rc) {
            rc = snprintf(m_message, MESSAGE_MAX_LEN, "(failed to format message) %s", format);
        }

        if (0 < rc) {
            m_ZLogFunc(level, file, line, func);
        }
    }
}


/******************************************************************************
 *                                                         Internal functions */
static inline void ZLog_StdFile(FILE *outfile, ZLogLevel_t level, const char *file, int line,
                                const char *func) {
    fprintf(outfile, "%s: [%s] %s:%d:%s: %s\n",
            m_moduleName, ZL_STR(level), file, line, func, m_message);
}

static void ZLog_StdOut(ZLogLevel_t level, const char *file, int line, const char *func) {
    ZLog_StdFile(stdout, level, file, line, func);
}

static void ZLog_StdErr(ZLogLevel_t level, const char *file, int line, const char *func) {
    ZLog_StdFile(stderr, level, file, line, func);
}

#ifdef Z_CHECK_HAS_SYSLOG
static void ZLog_Syslog(ZLogLevel_t level, const char *file, int line, const char *func) {
    syslog(ZL2SYSLOG_LEVEL(level), "[%s] %s:%d:%s: %s", ZL_STR(level), file, line, func, m_message);
}
#endif

