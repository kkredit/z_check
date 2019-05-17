/**
 * \file z_check.h
 *
 * \brief Define a set of logging and error handling macros.
 * \details
 * z_check is a library designed to provide a minimalist error handling
 * approach that enables unobtrusive yet rigorous error checking in C. It lives
 * at https://github.com/kkredit/z_check .
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 * \license MIT
 */

#ifndef Z_CHECK_H
#define Z_CHECK_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 *                                                                  REFERENCE *
 ******************************************************************************/
/**
 * ASSERTS
 *      Z_CT_ASSERT(condition)
 *      Z_RT_ASSERT(condition, message...)
 *
 * LOGS
 *      Z_LOG(level, message...)
 *      Z_LOG_IF(condition, level, message...)
 *
 * CHECKS
 *      Z_CHECK(condition, new_status, level, message...)
 *
 * DEBUG MACROS: for the above, replace "Z_" with "ZD_" for -DDEBUG only behavior
 *
 * WARNING SUPRESSORS
 *      UNUSED_VARIABLE(var)
 *      LABEL_UNUSED
 *
 * LOG LEVELS
 *      Z_EMERG     system is unusable
 *      Z_ALERT     action must be taken immediately
 *      Z_CRIT      critical conditions
 *      Z_ERR       error conditions
 *      Z_WARN      warning conditions
 *      Z_NOTICE    normal but significant condition
 *      Z_INFO      informational
 *      Z_DEBUG     debug-level messages
 *
 * LOG TARGETS
 *      Z_STDOUT    same as printf()
 *      Z_STDERR
 *      Z_SYSLOG    if configured
 *
 * METHODS
 *      void ZLog_Open(ZLogType_t logType, ZLogLevel_t logLevel, const char *moduleName)
 *      void ZLog_Close(void)
 *      void ZLog_LevelSet(ZLogLevel_t logLevel)
 *      void ZLog_LevelReset(void)
 */

/******************************************************************************
 *                                                                     SOURCE *
 ******************************************************************************/


/******************************************************************************
 *                                                                 Inclusions */
#include <string.h>
#include <assert.h>
#include <errno.h>


/******************************************************************************
 *                                                              Configuration */
#define Z_CHECK_HAS_SYSLOG

#define Z_CHECK_STATIC_CONFIG
#ifdef Z_CHECK_STATIC_CONFIG
    #undef Z_CHECK_HAS_SYSLOG

    #define Z_STDOUT    0   /* same as printf() */
    #define Z_STDERR    1

    #define Z_CHECK_MODULE_NAME        "main"
    #define Z_CHECK_LOG_FUNC           Z_STDOUT
    #define Z_CHECK_INIT_LOG_LEVEL     Z_DEBUG
#endif /* Z_CHECK_STATIC_CONFIG */


/******************************************************************************
 *                                                                    Helpers */
/**
 * \brief Layers of macros to support static asserts
 *
 * \details
 * Messages end up looking like
 *   "error: size of array ‘static_assertion_at_line_27’ is negative"
 * followed by the chain of "in expansion of macro..."s.
 */
#define Z_CT_ASSERT_GUTS_LOC(cond, loc) \
    do { \
        typedef char static_assertion##loc[(!!(cond))*2-1] __attribute__((unused)); \
    } while(0)
#define Z_CT_ASSERT_GUTS_TOKEN_NONSENSE(cond,line) Z_CT_ASSERT_GUTS_LOC(cond,_at_line_##line)
#define Z_CT_ASSERT_GUTS_LINE(cond,line) Z_CT_ASSERT_GUTS_TOKEN_NONSENSE(cond,line)
#define Z_CT_ASSERT_GUTS(cond) Z_CT_ASSERT_GUTS_LINE(cond, __LINE__)

/**
 * \brief Instead of getting the full path, get just the filename
 */
#define __FILENAME__ ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )


/******************************************************************************
 *                                                                    Defines */
#define Z_CT_ASSERT(condition) Z_CT_ASSERT_GUTS(condition)

#define Z_RT_ASSERT(condition, ...) \
    do { \
        if (!(condition)) { \
            Z_LOG(Z_EMERG, "Z_RT_ASSERT(" #condition ") failed!"); \
            Z_LOG(Z_EMERG, __VA_ARGS__); \
            assert(condition); \
            /* if get this far, NDEBUG is defined, meaning assert()s do not abort */ \
            Z_LOG(Z_ALERT, "assert() is disabled, so continuing despite failed assertion."); \
        } \
    } while(0)

/**
 * \brief Log a message
 */
#define Z_LOG(level, ...) \
    ZLog(level, __FILENAME__, __LINE__, __func__, __VA_ARGS__)

/**
 * \brief Conditionally log a message
 */
#define Z_LOG_IF(condition, level, ...) \
    do { \
        if (condition) { \
            Z_LOG(level, __VA_ARGS__); \
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
 * \param[IN]   ZLogLevel_t level: the importance level of the error
 * \param[IN]   __VA_ARGS__: the formatted error message
 */
#define Z_CHECK(condition, new_status, level, ...) \
    do { \
        if (condition) { \
            Z_LOG(level, __VA_ARGS__); \
            status = new_status; \
            goto cleanup; \
        } \
    } while(0)

/**
 * \brief Prevent warnings when compiling with -Wunused-label
 *
 * Usage:
 *  some-unused-label:
 *      LABEL_UNUSED;
 */
#define LABEL_UNUSED __attribute__((unused))

/**
 * \brief Prevent warnings when compiling with -Wunused-variable
 */
#define UNUSED_VARIABLE(var) (void)var

/**
 * \brief Debug versions of each macro
 *
 * Use same compilation flag as assert(): `NDEBUG`
 */
#ifdef NDEBUG
static inline void _macro_unused(const int dummy, ...) {UNUSED_VARIABLE(dummy);}
#define ZD_CT_ASSERT(...)   _macro_unused(__VA_ARGS__)
#define ZD_RT_ASSERT(...)   _macro_unused(__VA_ARGS__)
#define ZD_LOG(...)         _macro_unused(__VA_ARGS__)
#define ZD_LOG_IF(...)      _macro_unused(__VA_ARGS__)
#define ZD_CHECK(...)       _macro_unused(__VA_ARGS__)
#else
#define ZD_CT_ASSERT    Z_CT_ASSERT
#define ZD_RT_ASSERT    Z_RT_ASSERT
#define ZD_CHECK        Z_CHECK
#define ZD_LOG          Z_LOG
#define ZD_LOG_IF       Z_LOG_IF
#endif

/******************************************************************************
 *                                                                      Types */

/* Levels modeled after syslog */
typedef enum ZLogLevel_e
{
    Z_EMERG = 0,  /* system is unusable */
    Z_ALERT,      /* action must be taken immediately */
    Z_CRIT,       /* critical conditions */
    Z_ERR,        /* error conditions */
    Z_WARN,       /* warning conditions */
    Z_NOTICE,     /* normal but significant condition */
    Z_INFO,       /* informational */
    Z_DEBUG       /* debug-level messages */
} ZLogLevel_t;

#ifndef Z_CHECK_STATIC_CONFIG
typedef enum ZLogType_e
{
    Z_STDOUT = 0, /* same as printf() */
    Z_STDERR,
#ifdef Z_CHECK_HAS_SYSLOG
    Z_SYSLOG,
#endif
} ZLogType_t;
#endif


/******************************************************************************
 *                                                      Function declarations */

#ifndef Z_CHECK_STATIC_CONFIG
/**
 * \brief Opens and initializes the logger
 *
 * \post Logging is available
 *
 * \param[IN]   ZLogType_t logType: Desired log type
 * \param[IN]   ZLogLevel_t logLevel: Desired log level (inclusive)
 * \param[IN]   const char * moduleName: Name of module
 */
void ZLog_Open(ZLogType_t logType, ZLogLevel_t logLevel, const char *moduleName);

/**
 * \brief Closes and deconstructs the logger
 *
 * \pre Open logger with ZLog_Open
 * \post Logging is no longer available
 */
void ZLog_Close(void);
#endif /* Z_CHECK_STATIC_CONFIG */

/**
 * \brief Set the log level
 *
 * \param[IN]   ZLogLevel_t logLevel: Desired log level (inclusive)
 */
void ZLog_LevelSet(ZLogLevel_t logLevel);

/**
 * \brief Reset the log level to the original value
 */
void ZLog_LevelReset(void);

/**
 * \brief Write to the log
 *
 * \pre Logger must be intialized with ZLog_Open
 *
 * \param[IN]   ZLogLevel_t level: Error level of message
 * \param[IN]   const char * file: File where log is called
 * \param[IN]   int line: Line where log is called
 * \param[IN]   const char * func: Function where log is called
 * \param[IN]   const char * format: Error message
 */
void ZLog(ZLogLevel_t level, const char *file, int line, const char *func,
          const char *format, ...) __attribute__((format(printf, 5, 6)));


/******************************************************************************
 *                                                                        EOF */
#ifdef __cplusplus
}
#endif
#endif /* header guard */

