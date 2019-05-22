/**
 * \file example.c
 *
 * \brief Provides basic z_check example usage.
 * \details
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 * \license MIT
 */


/******************************************************************************
 *                                                                 Inclusions */
#include "z_check.h"
#include <stdio.h>
#include <stdbool.h>


/******************************************************************************
 *                                                                    Defines */

/* Z_CT_ASSERT_DECL performs compile-time asserts. Naturally, you can only use it to test
 * information known to the compiler during the build.
 *
 * Note that Z_CT_ASSERT_DECL is for use outside of functions. Z_CT_ASSERT_CODE is for use in
 * functions.
 *
 * Uncomment the second statement to see it in action. */
Z_CT_ASSERT_DECL(sizeof(int) == sizeof(unsigned));
//Z_CT_ASSERT_DECL(sizeof(int) == sizeof(long));


/******************************************************************************
 *                                                      Function declarations */
static int testExampleAsserts(void);
static int testExampleLogs(void);
static int testExampleChecks(void);


/******************************************************************************
 *                                                         External functions */
int main(void) {
    int status = 0;

    /* Open the logger.
     *
     * z_log supports static as well as dynamic configuration. The Open() and Close() functions
     * are for dynamic (runtime) configuration. To try dynamic configuration, un-comment-out
     * `ZLog_Open()` and `ZLog_cClose()`, then un-define `Z_CHECK_STATIC_CONFIG` in z_check.h. */
    //ZLog_Open(Z_STDOUT, Z_INFO, "example_dynamic");

    /* Try out the features. */
    status = testExampleAsserts();
    Z_CHECK(0 != status, -1, Z_ERR, "[X] testExampleAsserts failed!");

    status = testExampleLogs();
    Z_CHECK(0 != status, -1, Z_ERR, "[X] testExampleLogs failed!");

    status = testExampleChecks();
    Z_CHECK(0 != status, -1, Z_ERR,
             "[+] testExampleChecks failed! (as expected) status = %d", status);

    /* Because testExampleChecks() is expected to fail and the action is set to goto cleanup,
     * this message will not print. */
    Z_LOG(Z_ERR, "[X] this will not print");

    /* Use 'cleanup' tag to mark the end of the function, including actual clean up code. */
cleanup:
    Z_LOG_IF(0 != status, Z_INFO, "[+] returning");
    //ZLog_Close();
    return status;
}


/******************************************************************************
 *                                                         Internal functions */
int testExampleAsserts(void) {
    int status = 0;

    /* Z_CT_ASSERT_CODE performs compile-time asserts. Naturally, you can only use it to test
     * information known to the compiler during the build.
     *
     * Note that Z_CT_ASSERT_CODE is for use in functions. Z_CT_ASSERT_DECL is for use outside of
     * functions.
     *
     * Uncomment the second statement to see it in action. */

    Z_CT_ASSERT_CODE(2 + 2 == 4);
    //Z_CT_ASSERT_CODE(2 + 2 == 5);
    ZD_CT_ASSERT_CODE(2 + 2 == 4);

    /* Z_RT_ASSERT performs run-time asserts. It is a wrapper around assert() that allows
     * you to define human friendly messages while debugging.
     *
     * Uncomment the second statement to see it in action. */

    Z_RT_ASSERT(2 + 2 == 4, "2 + 2 == 4.");
    //Z_RT_ASSERT(2 + 2 == 5, "2 + 2 != 5. O cruel, needless misunderstanding!");
    ZD_RT_ASSERT(2 + 2 == 4, "2 + 2 == 4.");

    return status;
}

int testExampleLogs(void) {
    int status = 0;

    /* Try out a simple message, then some conditionals. */

    Z_LOG(Z_INFO, "[+] hello, log");
    ZD_LOG(Z_INFO, "[+] hello, debug log! (this is unrelated to Z_DEBUG and log levels)");

    Z_LOG_IF(false, Z_INFO, "[X] this will not print");
    Z_LOG_IF(true, Z_INFO, "[+] this will print");
    ZD_LOG_IF(true, Z_INFO, "[+] this will print when NDEBUG is not defined");


    /* You can change the log level during runtime, which makes avoiding noise much easier.
     * This works regardless of run-time or compile-time library configuration. */

    Z_LOG(Z_DEBUG, "[X] will not print");
    ZLog_LevelSet(Z_DEBUG);
    Z_LOG(Z_DEBUG, "[+] will print!");
    ZLog_LevelReset();
    Z_LOG(Z_DEBUG, "[X] will not print");

    return status;
}

int testExampleChecks(void) {
    int status = 0;
    int rvOfSomeOperation;

    /* Try out the Z_CHECK macro.
     *
     * Note that it checks if the condition is TRUE. You're checking for the error condition,
     * not asserting the non-error condition. */

    rvOfSomeOperation = 0;
    Z_CHECK(0 != rvOfSomeOperation, -1, Z_ERR, "[X] this not will occur");

    Z_CHECK(false, -1, Z_ERR, "[X] this will not occur");

    /* This will be tested when NDEBUG is not defined. */
    ZD_CHECK(false, -1, Z_ERR, "[X] this not will occur");

    /* This will occur. */
    Z_CHECK(2 + 2 == 4, -1, Z_ERR, "[+] this will fail");

cleanup:
    return status;
}

