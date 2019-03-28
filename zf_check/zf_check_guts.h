/**
 * \file zf_check_guts.h
 *
 * \brief Defines internal macros and objects for zf_check.
 * \details
 * NOT to be used directly by code bases using zf_check. Designed for internal
 * use only.
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 * \license MIT
 */

#ifndef ZF_CHECK_GUTS_H
#define ZF_CHECK_GUTS_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 *                                                                    Defines */

/**
 * \brief Instead of getting the full path, get just the filename
 */
#define __FILENAME__ ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )

/**
 * \brief Layers of macros to support static asserts
 *
 * \details
 * Messages end up looking like
 *   "error: size of array ‘static_assertion_at_line_27’ is negative"
 * followed by the chain of "in expansion of macro..."s.
 */
#define ZF_CT_ASSERT_GUTS_LOC(cond, loc) \
    do { \
        typedef char static_assertion##loc[(!!(cond))*2-1] __attribute__((unused)); \
    } while(0)
#define ZF_CT_ASSERT_GUTS_TOKEN_NONSENSE(cond,line) ZF_CT_ASSERT_GUTS_LOC(cond,_at_line_##line)
#define ZF_CT_ASSERT_GUTS_LINE(cond,line) ZF_CT_ASSERT_GUTS_TOKEN_NONSENSE(cond,line)
#define ZF_CT_ASSERT_GUTS(cond) ZF_CT_ASSERT_GUTS_LINE(cond, __LINE__)

/**
 * \brief ZF_CHECK helper; log message and assignment var
 */
#define ZF_CHECK_COMMON(setvar, newval, level, ...) \
    do { \
        ZFC_LOG(level, __VA_ARGS__); \
        setvar = newval; \
    } while(0)

/**
 * \brief The "goto" version of ZF_CHECK
 *
 * \post If condition is true, ZF_CHECK_COMMON is called and execution jumps to
 *       the 'goto' label
 *
 * \param[IN]   bool condition: statement that evaluates to true if error is
 *                      present
 * \param[IN]   [integral type] setvar: the variable that gets assigned
 * \param[IN]   [integral type] newval: the value that gets assigned
 * \param[IN]   ZfLogLevel_t level: the importance level of the error
 * \param[IN]   ...: the formatted error message
 */
#define ZF_CHECK_EXT_GOTO(condition, label, setvar, newval, level, ...) \
    do { \
        if (condition) { \
            ZF_CHECK_COMMON(setvar, newval, level, __VA_ARGS__); \
            goto label; \
        } \
    } while(0)

/**
 * \brief The "continue" version of ZF_CHECK
 *
 * \post If condition is true, ZF_CHECK_COMMON is called
 *
 * \param[IN]   bool condition: statement that evaluates to true if error is
 *                      present
 * \param[IN]   [integral type] setvar: the variable that gets assigned
 * \param[IN]   [integral type] newval: the value that gets assigned
 * \param[IN]   ZfLogLevel_t level: the importance level of the error
 * \param[IN]   ...: the formatted error message
 */
#define ZF_CHECK_EXT_CONT(condition, setvar, newval, level, ...) \
    do { \
        if (condition) { \
            ZF_CHECK_COMMON(setvar, newval, level, __VA_ARGS__); \
        } \
    } while(0)


/******************************************************************************
 *                                                                        EOF */
#ifdef __cplusplus
}
#endif
#endif /* header guard */

