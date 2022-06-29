/*
 * C-Ware License
 * 
 * Copyright (c) 2022, C-Ware
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Redistributions of modified source code must append a copyright notice in
 *    the form of 'Copyright <YEAR> <NAME>' to each modified source file's
 *    copyright notice, and the standalone license file if one exists.
 * 
 * A "redistribution" can be constituted as any version of the source code
 * that is intended to comprise some other derivative work of this code. A
 * fork created for the purpose of contributing to any version of the source
 * does not constitute a truly "derivative work" and does not require listing.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CWARE_CATALYST_PARSERS_H
#define CWARE_CATALYST_PARSERS_H

#define FILE_PATH_LENGTH    256 + 1

/* Limits */
#define TESTCASE_PATH   256 + 1
#define JOB_NAME        32 + 1
#define MAKE_PATH       128 + 1

#define QUALIFIER_NAME_LENGTH   32 + 1

/* Enumerations */
#define QUALIFIER_UNKNOWN   0
#define QUALIFIER_JOB       1
#define QUALIFIER_TESTCASE  2

#define QUALIFIER_JOB_NAME          1
#define QUALIFIER_JOB_MAKE          2
#define QUALIFIER_JOB_ARGUMENTS     3

/* Data structure properties */

/*
 * @docgen structure
 * @brief: input to give to a test
 * @name: Testcase
 *
 * @field path[TESTCASE_PATH + 1]: the path to the test
 * @type: char
 *
 * @field argv: an array of arguments to the program
 * @type: struct CStrings *
 *
 * @field input: the stdin to give the program
 * @type: struct CString 
 *
 * @field output: the stdout to expect
 * @type: struct CString 
 *
 * @field timeout: the timeout for the program to end in milliseconds
 * @type: int
*/
struct Testcase {
    char path[TESTCASE_PATH + 1];
    struct CStrings *argv;
    struct CString input;
    struct CString output;
    int timeout;
};

/*
 * @docgen: structure
 * @brief: container of test cases to run
 * @name: Testcases
 * 
 * @field length: the length of test cases
 * @type: int
 *
 * @field capacity: the length of test cases
 * @type: int
 *
 * @field contents: the test cases
 * @type: struct Testcase
*/
struct Testcases {
    int length;
    int capacity;
    struct Testcase *contents;
};

/*
 * @docgen: structure
 * @brief: a make(1) job that catalyst executes
 * @name: Job
 *
 * @field name[JOB_NAME + 1]: the name of the job
 * @type: char
 *
 * @field make_path[MAKE_PATH + 1]: the path to the make(1) binary
 * @type: char
 *
 * @field make_arguments: array of arguments to pass to make(1)
 * @type: struct CStrings *
*/
struct Job {
    char name[JOB_NAME + 1];
    char make_path[MAKE_PATH + 1];
    struct CStrings *make_arguments;
};

/*
 * @docgen: structure
 * @brief: array of make(1) jobs that catalyst executes
 * @name: Jobs
 *
 * @field length: the length of the array
 * @type: int
 *
 * @field capacity: the capacity of the array
 * @type: int
 *
 * @field contents: the jobs in the array
 * @type: struct Jobs *
*/
struct Jobs {
    int length;
    int capacity;
    struct Jobs *contents;
};

/*
 * @docgen: structure
 * @brief: a container for the parsed jobs and testcases
 * @name: Configuration
 *
 * @field jobs: the parsed jobs
 * @type: struct Jobs
 *
 * @field testcases: the parsed testcases
 * @type: struct Testcases
*/
struct Configuration {
    struct Jobs jobs;
    struct Testcases testcases;
};

/*
 * @docgen: structure
 * @brief: data that the parser uses in the parsing process.
 * @name: ParserState
 *
 * @field line: a line buffer that is reused in parsing
 * @type: struct CString
*/
struct ParserState {
    struct CString line;
};

/*
 * @docgen: function
 * @brief: parse a configuration file
 * @name: parse_configuration
 *
 * @include: parsers.h
 *
 * @description
 * @This function will parse the configuration file given
 * @to it, and extract all the testcases inside of it, and
 * @all of the jobs that are defined in the configuration
 * @file.
 * @description
 *
 * @error: path is NULL
 *
 * @param path: the path to the configuration file
 * @type: const char *
 *
 * @return: the parsed configuration
 * @type: struct Configuration
*/
struct Configuration parse_configuration(const char *path);

#endif








