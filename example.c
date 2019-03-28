/**
 * \file example.c
 *
 * \brief Provides basic zf_check example usage.
 * \details
 *
 * \copyright Copyright (c) 2019, Kevin Kredit.
 */


/******************************************************************************
 *                                                                 Inclusions */
#include "zf_check.h"
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

    /* Open the logger. */
    ZfcLog_Open(Z_STDOUT, Z_INFO, "example");

    status = testExampleAsserts();
    ZF_CHECK(0 != status, -1, Z_ERR, "[X] testExampleAsserts failed!");

    status = testExampleLogs();
    ZF_CHECK(0 != status, -1, Z_ERR, "[X] testExampleLogs failed!");

    status = testExampleChecks();
    ZF_CHECK(0 != status, -1, Z_ERR, "[X] testExampleChecks failed!");

    status = testExampleCheckGs();
    ZF_CHECK(0 != status, -1, Z_ERR,
             "[+] testExampleCheckGs failed! (as expected) status = %d", status);

    /* Because testExampleCheckGs() is expected to fail and the action is set to goto cleanup,
     * this message will not print. */
    ZFC_LOG(Z_ERR, "[X] this will not print");

    /* Use 'cleanup' tag to mark the end of the function, including actual clean up code. */
 cleanup:
    ZFC_LOG_IF(0 != status, Z_INFO, "[+] returning");
    ZfcLog_Close();
    return status;
}


/******************************************************************************
 *                                                         Internal functions */
int testExampleAsserts(void) {
    int status = 0;

    /* ZF_CT_ASSERT performs compile-time asserts. Naturally, you can only use it to test
     * information known to the compiler during the build.
     *
     * Uncomment the second statement to see it in action. */

    ZF_CT_ASSERT(2 + 2 == 4);
    //ZF_CT_ASSERT(2 + 2 == 5);

    /* ZF_RT_ASSERT performs run-time asserts. It is a wrapper around assert() that allows
     * you to define human friendly messages while debugging.
     *
     * Uncomment the second statement to see it in action. */

    ZF_RT_ASSERT(4 == 4, "2 + 2 == 4.");
    //ZF_RT_ASSERT(2 + 2 == 5, "2 + 2 != 5. O cruel, needless misunderstanding!");

    return status;
}

int testExampleLogs(void) {
    int status = 0;

    /* Try out a simple message, then some conditionals. */

    ZFC_LOG(Z_INFO, "[+] hello, log");

    ZFC_LOG_IF(false, Z_INFO, "[X] this will not print");
    ZFC_LOG_IF(true, Z_INFO, "[+] this will print");


    /* You can change the log level during runtime, which makes avoiding noise much easier. */

    ZFC_LOG(Z_DEBUG, "[X] will not print");
    ZfcLog_LevelSet(Z_DEBUG);
    ZFC_LOG(Z_DEBUG, "[+] will print!");
    ZfcLog_LevelReset();
    ZFC_LOG(Z_DEBUG, "[X] will not print");

    return status;
}

int testExampleChecks(void) {
    int status = 0;
    int rvOfSomeOperation;

    /* Try out the ZF_CHECK macro.
     *
     * Note that it checks if the condition is TRUE. You're checking for the error condition,
     * not asserting the non-error condition. */

    rvOfSomeOperation = 0;
    ZF_CHECK(0 != rvOfSomeOperation, -1, Z_ERR, "[X] this not will occur");


    /* ZF_CHECKC is the same as ZF_CHECK, except it continues even if the condition is true. */
    rvOfSomeOperation = -1;
    ZF_CHECKC(0 != rvOfSomeOperation, status, Z_WARN,
              "[+] this will occur, but since ZF_CHECKC continues, processing will go on");

    ZF_CHECK(false, -1, Z_ERR, "[X] this will not occur");

 cleanup:
    return status;
}

int testExampleCheckGs(void) {
    int status = 0;

    /* Try out the ZF_CHECKG macro.
     *
     * This is the same as ZF_CHECK, except it lets you define the 'goto' tag. */

    /* ... some steps... */
    ZF_CHECKG(false, err1, -1, Z_ERR, "[X] this not will occur");
    /* ... some more steps... */
    ZF_CHECKG(true, err2, -2, Z_ERR, "[+] this will occur");

    return status;
 err2:
    /* err2 specific cleanup steps */
 err1:
    /* err1 specific cleanup steps */
    return status;
}

