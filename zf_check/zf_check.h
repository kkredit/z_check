/**
 * \file zf_check.h
 *
 * \brief Define a set of logging and error handling macros.
 * \details
 * zf_check is a library designed to provide a minimalist error handling
 * approach that enables unobtrusive yet rigorous error checking in C. It lives
 * at https://github.com/kkredit/zf_check .
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 */

#ifndef ZF_CHECK_H
#define ZF_CHECK_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 *                                                                 Inclusions */
#include "zf_check_guts.h"
#include <string.h>
#include <assert.h>


/******************************************************************************
 *                                                                    Defines */
#define ZF_CT_ASSERT(condition) ZF_CT_ASSERT_GUTS(condition)

#define ZF_RT_ASSERT(condition, ...) \
    do { \
        if (!(condition)) { \
            ZFC_LOG(Z_EMERG, "ZF_RT_ASSERT(" #condition ") failed!"); \
            ZFC_LOG(Z_EMERG, __VA_ARGS__); \
            assert(condition); \
            /* if get this far, NDEBUG is defined, meaning assert()s do not abort */ \
            ZFC_LOG(Z_ALERT, "assert() is disabled, so continuing despite failed assertion."); \
        } \
    } while(0)

/**
 * \brief A macro to provide clean error checking code
 *
 * \pre Requires previous declaration of '[integral type] status' and a
 *      'cleanup' label
 * \post Status may be reassigned, function may continue or goto cleanup
 *
 * \param[IN]   bool condition: statement that evaluates to true if error is
 *                      present
 * \param[IN]   [integral type] new_status: the new status that gets assigned
 * \param[IN]   ZfLogLevel_t level: the importance level of the error
 * \param[IN]   ...: the formatted error message
 */
#define ZF_CHECK(condition, new_status, level, ...) \
    ZF_CHECK_EXT_GOTO(condition, cleanup, status, new_status, level, __VA_ARGS__);

/**
 * \brief A variants of ZF_CHECK that lets you name your own 'goto' label
 */
#define ZF_CHECKG(condition, label, new_status, level, ...) \
    ZF_CHECK_EXT_GOTO(condition, label, status, new_status, level, __VA_ARGS__)

/**
 * \brief A variant of ZF_CHECK that does not 'goto'
 */
#define ZF_CHECKC(condition, new_status, level, ...) \
    ZF_CHECK_EXT_CONT(condition, status, new_status, level, __VA_ARGS__)

/**
 * \brief Log a message
 */
#define ZFC_LOG(level, ...) \
    ZfcLog(level, __FILENAME__, __LINE__, __func__, __VA_ARGS__)

/**
 * \brief Conditionally log a message
 */
#define ZFC_LOG_IF(condition, level, ...) \
    do { \
        if (condition) { \
            ZFC_LOG(level, __VA_ARGS__); \
        } \
    } while(0)


/******************************************************************************
 *                                                                      Types */

/* Levels modeled after syslog */
typedef enum ZfLogLevel_e
{
    Z_EMERG = 0,  /* system is unusable */
    Z_ALERT,      /* action must be taken immediately */
    Z_CRIT,       /* critical conditions */
    Z_ERR,        /* error conditions */
    Z_WARN,       /* warning conditions */
    Z_NOTICE,     /* normal but significant condition */
    Z_INFO,       /* informational */
    Z_DEBUG       /* debug-level messages */
} ZfLogLevel_t;

typedef enum ZfLogType_e
{
    Z_ZFLOG = 0,
    Z_STDERR,
    Z_STDOUT, /* same as printf() */
    Z_SYSLOG
} ZfLogType_t;


/******************************************************************************
 *                                                      Function declarations */

/**
 * \brief Opens and initializes the logger
 *
 * \post Logging is available
 *
 * \param[IN]   ZfLogType_t logType: Desired log type
 * \param[IN]   ZfLogLevel_t logLevel: Desired log level (inclusive)
 * \param[IN]   const char * moduleName: Name of module
 */
void ZfcLog_Open(ZfLogType_t logType, ZfLogLevel_t logLevel, const char *moduleName);

/**
 * \brief Set the log level
 *
 * \param[IN]   ZfLogLevel_t logLevel: Desired log level (inclusive)
 */
void ZfcLog_LevelSet(ZfLogLevel_t logLevel);

/**
 * \brief Reset the log level to the original value
 */
void ZfcLog_LevelReset(void);

/**
 * \brief Closes and deconstructs the logger
 *
 * \pre Open logger with ZfcLog_Open
 * \post Logging is no longer available
 */
void ZfcLog_Close(void);

/**
 * \brief Write to the log
 *
 * \pre Logger must be intialized with ZfcLog_Open
 *
 * \param[IN]   ZfLogLevel_t level: Error level of message
 * \param[IN]   const char * file: File where log is called
 * \param[IN]   int line: Line where log is called
 * \param[IN]   const char * func: Function where log is called
 * \param[IN]   const char * format: Error message
 */
void ZfcLog(ZfLogLevel_t level, const char *file, int line, const char *func,
            const char *format, ...) __attribute__((format(printf, 5, 6)));


/******************************************************************************
 *                                                                        EOF */
#ifdef __cplusplus
}
#endif
#endif /* header guard */

