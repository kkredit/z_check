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
 *                                                      Function declarations */
static int testExampleAsserts(void);
static int testExampleLogs(void);
static int testExampleChecks(void);
static int testExampleCheckGs(void);


/******************************************************************************
 *                                                         External functions */
int main(void) {
    int status = 0;

    /* Open the logger.
     *
     * z_log supports static as well as dynamic configuration. The Open() and Close() functions
     * are for dynamic (runtime) configuration. To try static configuration, remove or comment-
     * out `ZfcLog_Open()` and `ZfcLog_cClose()`, then define `Z_CHECK_STATIC_CONFIG` in
     * z_check.h. */
    ZfcLog_Open(Z_STDOUT, Z_INFO, "example_dynamic");

    /* Try out the features. */
    status = testExampleAsserts();
    Z_CHECK(0 != status, -1, Z_ERR, "[X] testExampleAsserts failed!");

    status = testExampleLogs();
    Z_CHECK(0 != status, -1, Z_ERR, "[X] testExampleLogs failed!");

    status = testExampleChecks();
    Z_CHECK(0 != status, -1, Z_ERR, "[X] testExampleChecks failed!");

    status = testExampleCheckGs();
    Z_CHECK(0 != status, -1, Z_ERR,
             "[+] testExampleCheckGs failed! (as expected) status = %d", status);

    /* Because testExampleCheckGs() is expected to fail and the action is set to goto cleanup,
     * this message will not print. */
    Z_LOG(Z_ERR, "[X] this will not print");

    /* Use 'cleanup' tag to mark the end of the function, including actual clean up code. */
cleanup:
    Z_LOG_IF(0 != status, Z_INFO, "[+] returning");
    ZfcLog_Close();
    return status;
}


/******************************************************************************
 *                                                         Internal functions */
int testExampleAsserts(void) {
    int status = 0;

    /* Z_CT_ASSERT performs compile-time asserts. Naturally, you can only use it to test
     * information known to the compiler during the build.
     *
     * Uncomment the second statement to see it in action. */

    Z_CT_ASSERT(2 + 2 == 4);
    //Z_CT_ASSERT(2 + 2 == 5);
    Z_DCT_ASSERT(2 + 2 == 4);

    /* Z_RT_ASSERT performs run-time asserts. It is a wrapper around assert() that allows
     * you to define human friendly messages while debugging.
     *
     * Uncomment the second statement to see it in action. */

    Z_RT_ASSERT(2 + 2 == 4, "2 + 2 == 4.");
    //Z_RT_ASSERT(2 + 2 == 5, "2 + 2 != 5. O cruel, needless misunderstanding!");
    Z_DRT_ASSERT(2 + 2 == 4, "2 + 2 == 4.");

    return status;
}

int testExampleLogs(void) {
    int status = 0;

    /* Try out a simple message, then some conditionals. */

    Z_LOG(Z_INFO, "[+] hello, log");
    Z_DLOG(Z_INFO, "[+] hello, debug log! (this is unrelated to Z_DEBUG and log levels)");

    Z_LOG_IF(false, Z_INFO, "[X] this will not print");
    Z_LOG_IF(true, Z_INFO, "[+] this will print");
    Z_DLOG_IF(true, Z_INFO, "[+] this will print when NDEBUG is not defined");


    /* You can change the log level during runtime, which makes avoiding noise much easier.
     * This works regardless of run-time or compile-time library configuration. */

    Z_LOG(Z_DEBUG, "[X] will not print");
    ZfcLog_LevelSet(Z_DEBUG);
    Z_LOG(Z_DEBUG, "[+] will print!");
    ZfcLog_LevelReset();
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


    /* Z_CHECKC is the same as Z_CHECK, except it continues even if the condition is true. */
    rvOfSomeOperation = -1;
    Z_CHECKC(0 != rvOfSomeOperation, status, Z_WARN,
              "[+] this will occur, but since Z_CHECKC continues, processing will go on");

    Z_CHECK(false, -1, Z_ERR, "[X] this will not occur");

    /* These will be tested when NDEBUG is not defined. */
    Z_DCHECK(false, -1, Z_ERR, "[X] this not will occur");
    Z_DCHECKC(true, 0, Z_ERR, "[+] this will occur when NDEBUG is not defined, but is not fatal");

cleanup:
    return status;
}

int testExampleCheckGs(void) {
    int status = 0;

    /* Try out the Z_CHECKG macro.
     *
     * This is the same as Z_CHECK, except it lets you define the 'goto' tag. */

    /* e.g., malloc Thing1 */
    Z_CHECKG(false, err1, -1, Z_ERR, "[X] this not will occur");
    /* e.g., malloc Thing2 */
    Z_DCHECKG(false, err2, -1, Z_ERR, "[X] this will not occur");
    /* e.g., malloc Thing3 */
    Z_CHECKG(true, err3, -2, Z_ERR, "[+] this will occur");

    return status;
err3:
    /* err3 specific cleanup steps, e.g. free Thing3 */
err2:
    /* Since this label is referenced only when NDEBUG is not defined, when compiling with
     * -Wunused-label, LABEL_UNUSED is needed to prevent a compiler warning. */
    LABEL_UNUSED;

    /* err2 specific cleanup steps, e.g. free Thing2 */
err1:
    /* err1 specific cleanup steps, e.g. free Thing1 */
    return status;
}

