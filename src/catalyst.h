#ifndef CWARE_CATALYST_H
#define CWARE_CATALYST_H

#include "carray/carray.h"
#include "cstring/cstring.h"
#include "libpath/libpath.h"
#include "libproc/libproc.h"
#include "libmatch/libmatch.h"

struct Configuration;

/* Configuration */
#define CONFIGURATION_FILE  ".catalyst"
#define TESTS_DIRECTORY     "tests"

/* Useful macros */
#define INIT_VARIABLE(v) \
    memset(&(v), 0, sizeof((v)))

/*
 * @docgen: function
 * @brief: execute jobs and testcases
 * @name: handle_jobs
 *
 * @include: catalyst.h
 *
 * @description
 * @This function will first execute all testcases for each job that
 * @was parsed into the configuration. Jobs are sperate from testcases
 * @in that jobs tell Catalyst how to build the program. For each job,
 * @the output from the Makefile is redirected into a log file. The
 * @output of each job might not be necessary for the programmer to know,
 * @and so for each job that is performed, the user is notified of whether
 * @or not a job failed to compile the program, or if it succeeded. This
 * @way the user does not have to read through multiple (potentially large)
 * @log files just to see if a test failed to compile.
 * @description
*/
void handle_jobs(struct Configuration configuration);

#endif
