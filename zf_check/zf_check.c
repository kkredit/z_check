/**
 * \file
 *
 * \brief
 * \details
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 */


/******************************************************************************
 *                                                                 Inclusions */
#include "zf_check.h"
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


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


/******************************************************************************
 *                                                                      Types */
typedef void (*ZfcLogFn_t)(ZfLogLevel_t level, const char *file, int line, const char *func);


/******************************************************************************
 *                                                                       Data */
static const char *m_moduleName = NULL;
static ZfcLogFn_t m_zfcLogFunc = NULL;
static char m_message[MESSAGE_MAX_LEN] = { 0 };


/******************************************************************************
 *                                                      Function declarations */
static inline void ZfcLog_StdFile(FILE *outfile, ZfLogLevel_t level, const char *file, int line,
                                  const char *func);
static void ZfcLog_StdErr(ZfLogLevel_t level, const char *file, int line, const char *func);
static void ZfcLog_StdOut(ZfLogLevel_t level, const char *file, int line, const char *func);
static void ZfcLog_Syslog(ZfLogLevel_t level, const char *file, int line, const char *func);


/******************************************************************************
 *                                                         External functions */
void ZfcLog_Open(ZfLogType_t logType, const char *moduleName) {
    if (NULL != m_zfcLogFunc) {
        ZFC_LOG(Z_WARN, "called ZfcLog_Open() twice in same module, %s", m_moduleName);
    }
    else {
        m_moduleName = (NULL != moduleName) ? moduleName : DEFAULT_MODULE_NAME;

        switch (logType) {
            case Z_STDERR:
                m_zfcLogFunc = ZfcLog_StdErr;
                break;

            case Z_STDOUT:
                m_zfcLogFunc = ZfcLog_StdOut;
                break;

            case Z_SYSLOG:
                openlog(m_moduleName, LOG_CONS, LOG_LOCAL0);
                m_zfcLogFunc = ZfcLog_Syslog;
                break;

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
    if (ZfcLog_Syslog == m_zfcLogFunc) {
        closelog();
    }

    m_zfcLogFunc = NULL;
    m_moduleName = NULL;
    memset(m_message, 0, sizeof(m_message));
}

void ZfcLog(ZfLogLevel_t level, const char *file, int line, const char *func,
            const char *format, ...) {
    if (NULL == m_zfcLogFunc) {
        fprintf(stderr, "Error: May not use ZfcLog() before calling ZfcLog_Open()\n");
    }
    else {
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

static void ZfcLog_StdErr(ZfLogLevel_t level, const char *file, int line, const char *func) {
    ZfcLog_StdFile(stderr, level, file, line, func);
}

static void ZfcLog_StdOut(ZfLogLevel_t level, const char *file, int line, const char *func) {
    ZfcLog_StdFile(stdout, level, file, line, func);
}

static void ZfcLog_Syslog(ZfLogLevel_t level, const char *file, int line, const char *func) {
    syslog(ZFL2SYSLOG_LEVEL(level), "[%s] %s:%d:%s: %s", ZFL_STR(level), file, line, func, m_message);
}

