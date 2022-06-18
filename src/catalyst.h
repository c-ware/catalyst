#ifndef CWARE_CATALYST_H
#define CWARE_CATALYST_H

#include "carray/carray.h"
#include "cstring/cstring.h"
#include "libpath/libpath.h"

/* Configuration */
#define CONFIGURATION_FILE  ".catalyst"

/* Data structure properties */

/*
 * @docgen structure
 * @brief: input to give to a test
 * @name: Testcase
*/
struct Testcase {
    
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

#endif
