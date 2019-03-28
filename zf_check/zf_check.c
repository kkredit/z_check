/**
 * \file zf_check.c
 *
 * \brief Implement zf_check library logging functionality.
 * \details
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 * \license MIT
 */


/******************************************************************************
 *                                                                 Inclusions */
#include "zf_check.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef ZF_CHECK_HAS_ZF_LOG
#define ZF_LOG_DEF_LEVEL ZF_LOG_VERBOSE
#include "zf_log/zf_log.h"
#endif
#ifdef ZF_CHECK_HAS_SYSLOG
#include <syslog.h>
#endif


/******************************************************************************
 *                                                                    Defines */
#define DEFAULT_MODULE_NAME "Unnamed Module"
#define MESSAGE_MAX_LEN 512
#define ZFL_STR(level) \
    (Z_EMERG  == (level) ? "EMERGENCY" : \
     (Z_ALERT  == (level) ? "ALERT" : \
      (Z_CRIT   == (level) ? "CRITICAL" : \
       (Z_ERR    == (level) ? "ERROR" : \
        (Z_WARN   == (level) ? "WARNING" : \
         (Z_NOTICE == (level) ? "NOTICE" : \
          (Z_INFO   == (level) ? "INFO" : \
           (Z_DEBUG  == (level) ? "DEBUG" : "UNKNOWN_LEVEL"))))))))
#define ZFL2SYSLOG_LEVEL(level) ((int)level)
#define ZFL2ZFLOG_LEVEL(level) \
    (Z_EMERG  == (level) ? ZF_LOG_FATAL : \
     (Z_ALERT  == (level) ? ZF_LOG_FATAL : \
      (Z_CRIT   == (level) ? ZF_LOG_ERROR : \
       (Z_ERR    == (level) ? ZF_LOG_ERROR : \
        (Z_WARN   == (level) ? ZF_LOG_WARN : \
         (Z_NOTICE == (level) ? ZF_LOG_INFO : \
          (Z_INFO   == (level) ? ZF_LOG_DEBUG : \
           (Z_DEBUG  == (level) ? ZF_LOG_VERBOSE : ZF_LOG_NONE))))))))


/******************************************************************************
 *                                                                      Types */
typedef void (*ZfcLogFn_t)(ZfLogLevel_t level, const char *file, int line, const char *func);


/******************************************************************************
 *                                                                       Data */
#ifndef ZF_CHECK_STATIC_CONFIG

/* Dynamically configured */
static const char *m_moduleName = NULL;
static ZfcLogFn_t m_zfcLogFunc = NULL;
static ZfLogLevel_t m_logLevel;
static ZfLogLevel_t m_logLevelOrig;

#define FUNC_USED

#else /* ZF_CHECK_STATIC_CONFIG */

/* Statically configured */
#if !defined(ZF_CHECK_MODULE_NAME) || !defined(ZF_CHECK_LOG_FUNC) || !defined(ZF_CHECK_INIT_LOG_LEVEL)
    #error "Must fully define ZF_CHECK static configuration."
#endif
#if defined(ZF_CHECK_HAS_SYSLOG)
    #error "Syslog version of ZF_CHECK requires dynamic configuration"
#endif

#define FUNC_USED  __attribute__((unused))

static const char *m_moduleName = ZF_CHECK_MODULE_NAME;
#if ZF_CHECK_LOG_FUNC == Z_STDOUT
    #define m_zfcLogFunc ZfcLog_StdOut
#elif ZF_CHECK_LOG_FUNC == Z_STDERR
    #define m_zfcLogFunc ZfcLog_StdErr
#elif ZF_CHECK_LOG_FUNC == Z_ZFLOG
    #define m_zfcLogFunc ZfcLog_ZfLog
#else
    #error "invalid ZF_CHECK_LOG_FUNC"
#endif
static ZfLogLevel_t m_logLevel = ZF_CHECK_INIT_LOG_LEVEL;
static ZfLogLevel_t m_logLevelOrig = ZF_CHECK_INIT_LOG_LEVEL;

#endif /* ZF_CHECK_STATIC_CONFIG */

static char m_message[MESSAGE_MAX_LEN] = { 0 };


/******************************************************************************
 *                                                      Function declarations */
static inline void ZfcLog_StdFile(FILE *outfile, ZfLogLevel_t level, const char *file, int line,
                                  const char *func);
static void ZfcLog_StdErr(ZfLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
static void ZfcLog_StdOut(ZfLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
#ifdef ZF_CHECK_HAS_ZF_LOG
static void ZfcLog_ZfLog(ZfLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
#endif
#ifdef ZF_CHECK_HAS_SYSLOG
static void ZfcLog_Syslog(ZfLogLevel_t level, const char *file, int line, const char *func) FUNC_USED;
#endif


/******************************************************************************
 *                                                         External functions */
#ifndef ZF_CHECK_STATIC_CONFIG
void ZfcLog_Open(ZfLogType_t logType, ZfLogLevel_t logLevel, const char *moduleName) {
    if (NULL != m_zfcLogFunc) {
        ZFC_LOG(Z_WARN, "called ZfcLog_Open() twice in same module, %s", m_moduleName);
    }
    else {
        m_moduleName = (NULL != moduleName) ? moduleName : DEFAULT_MODULE_NAME;
        m_logLevel = logLevel;
        m_logLevelOrig = logLevel;

        switch (logType) {
            case Z_STDERR:
                m_zfcLogFunc = ZfcLog_StdErr;
                break;

            case Z_STDOUT:
                m_zfcLogFunc = ZfcLog_StdOut;
                break;

#ifdef ZF_CHECK_HAS_ZF_LOG
            case Z_ZFLOG:
                m_zfcLogFunc = ZfcLog_ZfLog;
                break;
#endif

#ifdef ZF_CHECK_HAS_SYSLOG
            case Z_SYSLOG:
                openlog(m_moduleName, LOG_CONS, LOG_LOCAL0);
                m_zfcLogFunc = ZfcLog_Syslog;
                break;
#endif

            default:
                /* don't have ZFC_LOG setup yet to use */
                fprintf(stderr, "Warning: Unknown log type (%d); falling back to stderr\n",
                        (int)logType);
                m_zfcLogFunc = ZfcLog_StdErr;
                break;
        }
    }
}

void ZfcLog_Close(void) {
#ifdef ZF_CHECK_HAS_SYSLOG
    if (ZfcLog_Syslog == m_zfcLogFunc) {
        closelog();
    }
#endif

    m_zfcLogFunc = NULL;
    m_moduleName = NULL;
    memset(m_message, 0, sizeof(m_message));
}
#endif /* ZF_CHECK_STATIC_CONFIG */

void ZfcLog_LevelSet(ZfLogLevel_t logLevel) {
    m_logLevel = logLevel;
}

void ZfcLog_LevelReset(void) {
    m_logLevel = m_logLevelOrig;
}

void ZfcLog(ZfLogLevel_t level, const char *file, int line, const char *func,
            const char *format, ...) {
#ifndef ZF_CHECK_STATIC_CONFIG
    if (NULL == m_zfcLogFunc) {
        fprintf(stderr, "Error: May not use ZfcLog() before calling ZfcLog_Open()\n");
    }
    else
#endif /* ZF_CHECK_STATIC_CONFIG */
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
            m_zfcLogFunc(level, file, line, func);
        }
    }
}


/******************************************************************************
 *                                                      Function declarations */
static inline void ZfcLog_StdFile(FILE *outfile, ZfLogLevel_t level, const char *file, int line,
                                  const char *func) {
    fprintf(outfile, "%s: [%s] %s:%d:%s: %s\n",
            m_moduleName, ZFL_STR(level), file, line, func, m_message);
}

static void ZfcLog_StdOut(ZfLogLevel_t level, const char *file, int line, const char *func) {
    ZfcLog_StdFile(stdout, level, file, line, func);
}

static void ZfcLog_StdErr(ZfLogLevel_t level, const char *file, int line, const char *func) {
    ZfcLog_StdFile(stderr, level, file, line, func);
}

#ifdef ZF_CHECK_HAS_ZF_LOG
static void ZfcLog_ZfLog(ZfLogLevel_t level, const char *file, int line, const char *func) {
    ZF_LOG_WRITE(ZFL2ZFLOG_LEVEL(level), "", "%s:%d:%s: %s", file, line, func, m_message);
}
#endif

#ifdef ZF_CHECK_HAS_SYSLOG
static void ZfcLog_Syslog(ZfLogLevel_t level, const char *file, int line, const char *func) {
    syslog(ZFL2SYSLOG_LEVEL(level), "[%s] %s:%d:%s: %s", ZFL_STR(level), file, line, func, m_message);
}
#endif

