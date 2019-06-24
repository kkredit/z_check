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
#include <stdbool.h>
#ifdef Z_CHECK_HAS_SYSLOG
#include <syslog.h>
#endif


/******************************************************************************
 *                                                                    Defines */
#ifdef Z_CHECK_STATIC_CONFIG
    #define FUNC_MAYBE_UNUSED __attribute__((unused))
#else
    #define FUNC_MAYBE_UNUSED
    #define DEFAULT_MODULE_NAME "z_check"
#endif

#define PURE_FUNC __attribute__((pure))     /* no side effects, global memory read-only */
#define CONST_FUNC __attribute__((const))   /* no side effects, no global memory access */
#define MESSAGE_MAX_LEN 512
#define MAX_LEGAL_LEVEL ((unsigned)Z_DEBUG)


/******************************************************************************
 *                                                                      Types */
typedef void (*ZLogFn_t)(const ZLogLevel_t level, const char * const file, const int line,
                         const char * const func, const char * const message);


/******************************************************************************
 *                                                      Function declarations */
#ifndef Z_CHECK_STATIC_CONFIG
static void ZLog_ModuleNameInit(const char * const moduleName);
static ZLogLevel_t ZLog_LevelSanitize(const ZLogLevel_t logLevel);
static inline bool ZLog_LevelIsLegal(const ZLogLevel_t level) CONST_FUNC;
#endif

static inline bool ZLog_LevelPasses(const ZLogLevel_t level) PURE_FUNC;
static inline const char * ZLog_LevelStr(const ZLogLevel_t level) CONST_FUNC;
static inline void ZLog_StdFile(FILE *outfile, const ZLogLevel_t level, const char * const file,
                                const int line, const char * const func, const char * const message);
static void ZLog_StdErr(const ZLogLevel_t level, const char * const file, const int line,
                        const char * const func, const char * const message) FUNC_MAYBE_UNUSED;
static void ZLog_StdOut(const ZLogLevel_t level, const char * const file, const int line,
                        const char * const func, const char * const message) FUNC_MAYBE_UNUSED;
#ifdef Z_CHECK_HAS_SYSLOG
static inline int ZLog_Level2Syslog(const ZLogLevel_t level) CONST_FUNC;
static void ZLog_Syslog(const ZLogLevel_t level, const char * const file, const int line,
                        const char * const func, const char * const message);
#endif


/******************************************************************************
 *                                                                       Data */
#ifndef Z_CHECK_STATIC_CONFIG
    /* Dynamically configured */
    static char m_moduleName[Z_CHECK_MODULE_NAME_MAX_LEN] = {0};
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

    static const char * const m_moduleName = Z_CHECK_MODULE_NAME;

    #if Z_CHECK_LOG_FUNC == Z_STDOUT
        static const ZLogFn_t m_ZLogFunc = ZLog_StdOut;
    #elif Z_CHECK_LOG_FUNC == Z_STDERR
        static const ZLogFn_t m_ZLogFunc = ZLog_StdErr;
    #else
        #error "invalid Z_CHECK_LOG_FUNC"
    #endif

    Z_CT_ASSERT_DECL(Z_CHECK_INIT_LOG_LEVEL <= MAX_LEGAL_LEVEL);
    static ZLogLevel_t m_logLevel = Z_CHECK_INIT_LOG_LEVEL;
    static const ZLogLevel_t m_logLevelOrig = Z_CHECK_INIT_LOG_LEVEL;
#endif


/******************************************************************************
 *                                                         External functions */
#ifndef Z_CHECK_STATIC_CONFIG
void ZLog_Open(const ZLogType_t logType, const ZLogLevel_t logLevel, const char * const moduleName) {
    if (NULL != m_ZLogFunc) {
        Z_LOG(Z_WARN, "called ZLog_Open() twice in same module, %s", m_moduleName);
    }
    else {
        const ZLogLevel_t sanitizedLogLevel = ZLog_LevelSanitize(logLevel);
        ZLog_ModuleNameInit(moduleName);

        m_logLevel = sanitizedLogLevel;
        m_logLevelOrig = sanitizedLogLevel;

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
    memset(m_moduleName, 0, sizeof(m_moduleName));
}
#endif /* Z_CHECK_STATIC_CONFIG */

void ZLog_LevelSet(const ZLogLevel_t logLevel) {
    m_logLevel = logLevel;
}

void ZLog_LevelReset(void) {
    m_logLevel = m_logLevelOrig;
}

void ZLog(const ZLogLevel_t level, const char * const file, const int line, const char * const func,
          const char *format, ...) {
#ifndef Z_CHECK_STATIC_CONFIG
    if (NULL == m_ZLogFunc) {
        fprintf(stderr, "Error: May not use ZLog() before calling ZLog_Open()\n");
    }
    else
#endif

    if (ZLog_LevelPasses(level)) {
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
#ifndef Z_CHECK_STATIC_CONFIG
static void ZLog_ModuleNameInit(const char * const moduleName) {
    const char * const moduleNameToUse = (NULL != moduleName) ? moduleName : DEFAULT_MODULE_NAME;
    (void)strncpy(m_moduleName, moduleNameToUse, Z_CHECK_MODULE_NAME_MAX_LEN - 1);
}

static ZLogLevel_t ZLog_LevelSanitize(const ZLogLevel_t logLevel) {
    ZLogLevel_t sanitizedLevel = logLevel;
    if (!ZLog_LevelIsLegal(logLevel)) {
        /* don't have Z_LOG setup yet to use */
        fprintf(stderr, "Warning: Invalid log level (%u); falling back to MAX_LEGAL_LEVEL (%u)\n",
                (unsigned)logLevel, MAX_LEGAL_LEVEL);
        sanitizedLevel = MAX_LEGAL_LEVEL;
    }
    return sanitizedLevel;
}

static inline bool ZLog_LevelIsLegal(const ZLogLevel_t level) {
    return (MAX_LEGAL_LEVEL >= (unsigned)level);
}
#endif /* Z_CHECK_STATIC_CONFIG */

static inline bool ZLog_LevelPasses(const ZLogLevel_t level) {
    return ((unsigned)level <= (unsigned)m_logLevel);
}

static inline const char * ZLog_LevelStr(const ZLogLevel_t level) {
    /* Do not need a runtime assert that the log level is in range because of two checks:
     *  (1): levels > m_logLevel are thrown out in ZLog_LevelPasses()
     *  (2): m_logLevels > MAX_LEVEL_INDEX are thrown out in Z_CT_ASSERTs in static config and
     *          with input sanitization in dynamic config. */
    const char * const levelStrs[] = {
        "EMERGENCY",
        "ALERT",
        "CRITICAL",
        "ERROR",
        "WARNING",
        "NOTICE",
        "INFO",
        "DEBUG",
    };
    return levelStrs[(int)level];
}

static inline void ZLog_StdFile(FILE *outfile, const ZLogLevel_t level, const char * const file,
                                const int line, const char * const func, const char * const message) {
    fprintf(outfile, "%s: [%s] %s:%d:%s: %s\n",
            m_moduleName, ZLog_LevelStr(level), file, line, func, message);
}

static void ZLog_StdOut(const ZLogLevel_t level, const char * const file, const int line,
                        const char * const func, const char * const message) {
    ZLog_StdFile(stdout, level, file, line, func, message);
}

static void ZLog_StdErr(const ZLogLevel_t level, const char * const file, const int line,
                        const char * const func, const char * const message) {
    ZLog_StdFile(stderr, level, file, line, func, message);
}

#ifdef Z_CHECK_HAS_SYSLOG
static inline int ZLog_Level2Syslog(const ZLogLevel_t level) {
    return (int)level;
}

static void ZLog_Syslog(const ZLogLevel_t level, const char * const file, const int line,
                        const char * const func, const char * const message) {
    syslog(ZLog_Level2Syslog(level), "[%s] %s:%d:%s: %s",
           ZLog_LevelStr(level), file, line, func, message);
}
#endif

