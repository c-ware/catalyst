#ifndef CWARE_CATALYST_H
#define CWARE_CATALYST_H

#include "carray/carray.h"
#include "cstring/cstring.h"
#include "libpath/libpath.h"

/* Configuration */
#define CONFIGURATION_FILE  ".catalyst"

/* Limits */
#define TESTCASE_PATH   256 + 1
#define JOB_NAME        32 + 1
#define MAKE_PATH       128 + 1

/* Data structure properties */

/*
 * @docgen structure
 * @brief: input to give to a test
 * @name: Testcase
 *
 * @param path[TESTCASE_PATH + 1]: the path to the test
 * @type: char
 *
 * @param argv: an array of arguments to the program
 * @type: struct CStrings *
 *
 * @param stdin: the stdin to give the program
 * @type: struct CString 
 *
 * @param stdout: the stdout to expect
 * @type: struct CString 
 *
 * @param timeout: the timeout for the program to end in milliseconds
 * @type: int
*/
struct Testcase {
    char path[TESTCASE_PATH + 1];
    struct CStrings *argv;
    struct CString stdin;
    struct CString stdout;
    int timeout;
};

/*
 * @docgen: structure
 * @brief: container of test cases to run
 * @name: Testcases
 * 
 * @param length: the length of test cases
 * @type: int
 *
 * @param capacity: the length of test cases
 * @type: int
 *
 * @param contents: the test cases
 * @type: struct Testcase
*/
struct Testcases {
    int length;
    int capacity;
    struct Testcase *contents;
};

struct Job {
    char name[JOB_NAME + 1];
    char make_path[MAKE_PATH + 1];
    struct CStrings *make_arguments;
};

#endif
