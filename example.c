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


/******************************************************************************
 *                                                         External functions */
int main(void) {
    int status = 0;
    int ex = 0;

    /* Open the logger */
    ZfcLog_Open(Z_STDOUT, "example");

    /* Test out a simple message */
    ZFC_LOG(Z_INFO, "[1/4] hello, log");

    /* Try out some conditionals */
    ZFC_LOG_IF(0 != ex, Z_INFO, "[X] this will not print");
    ZF_CHECK(0 != ex, Z_CLEANUP, -1, Z_ERR, "[X] this not will occur");

    ZFC_LOG_IF(0 == ex, Z_INFO, "[2/4] this will print");
    ZF_CHECK(0 == ex, Z_CLEANUP, -1, Z_ERR, "[3/4] this will occur");

    /* Because the previous check occurred and the action was to goto cleanup,
     * this message will not print. */
    ZFC_LOG(Z_INFO, "[X] this will not print");

    /* Use cleanup tag to mark the end of the function, including actual clean up code. */
cleanup:
    ZFC_LOG_IF(0 != status, Z_INFO, "[4/4] returning");
    ZfcLog_Close();
    return status;
}

