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

#ifndef TPL_PUBLIC_H
#define	TPL_PUBLIC_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 *                                                                 Inclusions */
#include <string.h>


/******************************************************************************
 *                                                                    Defines */

/**
 * \brief A macro to provide clean error checking code
 *
 * \pre Requires previous declaration of '[integral type] status' and a
 *       'cleanup' label
 * \post Status may be reassigned, function may continue or goto cleanup
 *
 * \param[IN]   bool condition: statement that evaluates to true if error is
 *                      present
 * \param[IN]   ZfErrorAction_t action: determines how to respond in
 *                      case error condition is true
 * \param[IN]   [integral type] new_status: the new status that gets assigned
 * \param[IN]   ZfLogLevel_t level: the importance level of the error
 * \param[IN]   ...: the formatted error message
 */
#define ZF_CHECK(condition, action, new_status, level, ...) \
    do \
    { \
        if (condition) { \
            status = new_status; \
            ZFC_LOG(level, __VA_ARGS__); \
            switch (action) { \
                case Z_CLEANUP: \
                    goto cleanup; \
                    break; \
                default: \
                    break; \
            } \
        } \
    } while(0)


/**
 * \brief Instead of getting the full path, get just the filename
 */
#define __FILENAME__ ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )

#define ZFC_LOG(level, ...) \
    ZfcLog(level, __FILENAME__, __LINE__, __func__, __VA_ARGS__)

#define ZFC_LOG_IF(condition, level, ...) \
    do \
    { \
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
    Z_STDOUT,
    Z_SYSLOG,
} ZfLogType_t;

typedef enum ZfErrorAction_e
{
    Z_NONE = 0,
    Z_CLEANUP,
} ZfErrorAction_t;


/******************************************************************************
 *                                                      Function declarations */

/**
 * \brief Opens and initializes the logger
 *
 * \post Logging is available
 *
 * \param[IN]   ZfLogType_t logType: Desired log type
 * \param[IN]   const char * moduleName: Name of module
 */
void ZfcLog_Open(ZfLogType_t logType, const char *moduleName);

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

