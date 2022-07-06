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

#define QUALIFIER_NAME_LENGTH       32 + 1
#define JOB_KEY_NAME_LENGTH         32 + 1
#define TESTCASE_KEY_NAME_LENGTH    32 + 1
#define NUMBER_LENGTH               32 + 1
#define READ_BUFFER_LENGTH          12

/* Enumerations */
#define QUALIFIER_UNKNOWN   0
#define QUALIFIER_JOB       1
#define QUALIFIER_TESTCASE  2

#define QUALIFIER_JOB_NAME          1
#define QUALIFIER_JOB_MAKE          2
#define QUALIFIER_JOB_ARGUMENTS     3

#define QUALIFIER_TESTCASE_FILE         1
#define QUALIFIER_TESTCASE_NAME         2
#define QUALIFIER_TESTCASE_ARGV         3
#define QUALIFIER_TESTCASE_STDOUT       4
#define QUALIFIER_TESTCASE_STDIN        5
#define QUALIFIER_TESTCASE_TIMEOUT      6

/* Data structure properties */
#define TESTCASE_TYPE   struct Testcase
#define TESTCASE_HEAP   1

#define JOB_TYPE   struct Job
#define JOB_HEAP   1

#define HANDLE_EOF(_cursor)                                                                                    \
do {                                                                                                           \
    if((_cursor).cursor != (_cursor).length)                                                                   \
        break;                                                                                                 \
                                                                                                               \
    fprintf(stderr, "catalyst: failed to parse configuration file-- expected character on line %i, got EOF"    \
            " (%s:%i)\n", (_cursor).line + 1, __FILE__, __LINE__);                                             \
    exit(EXIT_FAILURE);                                                                                        \
} while(0)

#define ASSERT_NEXT_CHARACTER(cursor, character)                                                              \
do {                                                                                                          \
    int __NEXT_CHARACTER = libmatch_cursor_getch(cursor);                                                     \
                                                                                                              \
    if(__NEXT_CHARACTER == (character))                                                                       \
        break;                                                                                                \
                                                                                                              \
    if(character == LIBMATCH_EOF) {                                                                           \
        fprintf(stderr, "catalyst: failed to parse configuration file-- expected '%c' on line %i, got EOF\n", \
               (character), (cursor)->line + 1);                                                              \
        exit(EXIT_FAILURE);                                                                                   \
    }                                                                                                         \
                                                                                                              \
    fprintf(stderr, "catalyst: failed to parse configuration file-- expected '%c' on line %i, got '%c'\n",    \
            (character), (cursor)->line + 1, __NEXT_CHARACTER);                                               \
    exit(EXIT_FAILURE);                                                                                       \
} while(0)

/*
 * @docgen structure
 * @brief: input to give to a test
 * @name: Testcase
 *
 * @field path: the path to the test
 * @type: struct CString
 *
 * @field name: the name of the testcase
 * @type: struct CString
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
    struct CString path;
    struct CString name;
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
 * @field name: the name of the job
 * @type: struct CString
 *
 * @field make_path: the path to the make(1) binary
 * @type: struct CString
 *
 * @field make_arguments: array of arguments to pass to make(1)
 * @type: struct CStrings *
*/
struct Job {
    struct CString name;
    struct CString make_path;
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
 * @type: struct Job *
*/
struct Jobs {
    int length;
    int capacity;
    struct Job *contents;
};

/*
 * @docgen: structure
 * @brief: a container for the parsed jobs and testcases
 * @name: Configuration
 *
 * @field jobs: the parsed jobs
 * @type: struct Jobs *
 *
 * @field testcases: the parsed testcases
 * @type: struct Testcases *
*/
struct Configuration {
    struct Jobs *jobs;
    struct Testcases *testcases;
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

/*
 * @docgen: function
 * @brief: parse an integer from a value
 * @name: parse_uinteger
 *
 * @include: parsers.h
 *
 * @description
 * @This function, with the cursor on the first digit of a number,
 * @parse the number into a usable unsigned integer.
 * @description
 *
 * @error: cursor is NULL
 *
 * @param cursor: the cursor to use
 * @type: struct LibmatchCursor *
 *
 * @return: the parsed integer
 * @type: unsigned int
*/
unsigned int parse_uinteger(struct LibmatchCursor *cursor);

/*
 * @docgen: function
 * @brief: parse a string from a value
 * @name: parse_string
 *
 * @include: parsers.h
 *
 * @description
 * @This function, with the cursor on the opening quote of a string, will
 * @parse the string into a usable cstring object. This will interpret
 * @basic escape sequences.
 * @description
 *
 * @error: cursor is NULL
 *
 * @param cursor: the cursor to use
 * @type: struct LibmatchCursor *
 *
 * @return: the parsed cstring
 * @type: struct CString
*/
struct CString parse_string(struct LibmatchCursor *cursor);

/*
 * @docgen: function
 * @brief: parse an array of strings from a value
 * @name: parse_string_list
 *
 * @include: parsers.h
 *
 * @description
 * @This function, with the cursor on the opening quote of the first
 * @string, will begin parsing an array of strings.
 * @description
 *
 * @error: cursor is NULL
 *
 * @param cursor: the cursor to use
 * @type: struct LibmatchCursor *
 *
 * @return: the parsed array of cstrings
 * @type: struct CStrings
*/
struct CStrings *parse_string_list(struct LibmatchCursor *cursor);

#endif



















