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

/*
 * Functions for parsing a configuration file.
*/

#include <ctype.h>
#include <stdio.h>

#include "../catalyst.h"

#define HANDLE_EOF(cursor)                                                                                  \
do {                                                                                                        \
    if((cursor).cursor != (cursor).length)                                                                  \
        break;                                                                                              \
                                                                                                            \
    fprintf(stderr, "%s", "catalyst: failed to parse configuration file-- expected, character, got EOF\n"); \
    exit(EXIT_FAILURE);                                                                                     \
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
               (character), (cursor)->line);                                                                  \
        exit(EXIT_FAILURE);                                                                                   \
    }                                                                                                         \
                                                                                                              \
    fprintf(stderr, "catalyst: failed to parse configuration file-- expected '%c' on line %i, got '%c'\n",    \
            (character), (cursor)->line, __NEXT_CHARACTER);                                                   \
    exit(EXIT_FAILURE);                                                                                       \
} while(0)

/*
 * @docgen: function
 * @brief: perform error checks on a qualifier
 * @name: error_check_qualifier
 *
 * @description
 * @This function will perform error checking on the cursor
 * @based off of its current position when the function is
 * @called. The function will enforce the regular expression
 * @
 * @[a-zA-Z_][a-zA-Z0-9_]*: {
 * @
 * @From the current position of the cursor, to the end of
 * @the line it is on.
 * @
 * @description
 *
 * @notes
 * @This function will not validate the type of qualifier that the
 * @line contains. Currently, the only two valid qualifiers that
 * @the parser can understand is 'job' and 'testcase'. The caller
 * @will be responsible for verifying that the qualifier is an
 * @acceptable name.
 * @
 * @Also, as should be expected by this function not taking a
 * @pointer to the cursor, this function will not modify the
 * @caller's cursor, and will instead use a copy of it.
 * @notes
 *
 * @param cursor: the cursor to validate
 * @type: struct LibmatchCursor
*/
void error_check_qualifier_header(struct LibmatchCursor cursor) {
    int character = libmatch_cursor_getch(&cursor);

    HANDLE_EOF(cursor);

    /* First character must be in the character class [A-Za-z_] */
    if(strchr(LIBMATCH_ALPHA "_", character) == NULL) {
        fprintf(stderr, "catalyst: failed to parse configuration file-- first"
                " character of qualifier name on line %i must be alphabetical"
                " or an underscore.\n", cursor.line);
        exit(EXIT_FAILURE);
    }

    character = libmatch_cursor_getch(&cursor);

    /* All of the characters after the first one until a colon must be
     * alphanumeric, or an underscore. */
    while(1) {
        HANDLE_EOF(cursor);

        if(character == ':')
            break;

        if(strchr(LIBMATCH_ALPHANUM "_", character) == NULL) {
            fprintf(stderr, "catalyst: failed to parse configuration file-- every"
                    " character after the start of a qualifier name on line %i"
                    " must be alphanumerical, or an underscore.\n", cursor.line);
            exit(EXIT_FAILURE);
        }

        character = libmatch_cursor_getch(&cursor);
    }

    /* We only get to this point if the a colon was met, so we can
     * assume that a colon was found. Next character after it must
     * be a space. */
    ASSERT_NEXT_CHARACTER(&cursor, ' ');
    ASSERT_NEXT_CHARACTER(&cursor, '{');
}

/*
 * @docgen: function
 * @brief: enumerate the name of the qualifier
 * @name: enumerate_qualifier
 *
 * @description
 * @This function will transform the name of the qualifier that
 * @the cursor is sitting on into a numeric value that can be
 * @easily used by the parser.
 * @description
 *
 * @notes
 * @This function will advance the cursor that the caller passes
 * @to it, as that makes the parsing process smoother.
 * @notes
 *
 * @param cursor: the cursor to enumerate
 * @type: struct LibmatchCursor *
 *
 * @return: the enumerated value, or QUALIFIER_UNKNOWN
 * @type: int
*/
int enumerate_qualifier(struct LibmatchCursor *cursor) {
    int written = 0;
    char qualifier_name[QUALIFIER_NAME_LENGTH + 1] = "";

    written = libmatch_read_until(cursor, qualifier_name, QUALIFIER_NAME_LENGTH, ":");

    /* Make sure it did not overflow. */
    if(written >= QUALIFIER_NAME_LENGTH) {
        fprintf(stderr, "catalyst: qualifier name on line %i too long\n", cursor->line + 1);
        exit(EXIT_FAILURE);
    }

    if(strcmp(qualifier_name, "job") == 0)
        return QUALIFIER_JOB;

    if(strcmp(qualifier_name, "testcase") == 0)
        return QUALIFIER_TESTCASE;

    return QUALIFIER_UNKNOWN;
}

struct LibmatchCursor open_cursor_stream(const char *path) {
    FILE *stream = fopen(path, "r");
    struct LibmatchCursor cursor;

    INIT_VARIABLE(cursor);
    cursor = libmatch_cursor_from_stream(stream);
    fclose(stream);

    return cursor;
}

struct Configuration parse_configuration(const char *path) {
    struct LibmatchCursor cursor;
    struct Configuration configuration;

    liberror_is_null(parse_configuration, path);

    INIT_VARIABLE(cursor);
    INIT_VARIABLE(configuration);

    cursor = open_cursor_stream(path);

    /* Consume the file */
    while(cursor.cursor < cursor.length) {
        int qualifier = 0;
        int character = libmatch_cursor_getch(&cursor);

        /* Keep going until a printable character is found. Also, 0x20
         * is printable for some reason..? */
        if(isprint(character) == 0 || character == ' ')
            continue;

        /* Currently, the cursor is AFTER the first alphabrtical
         * character, which makes qualifier enumeration impossible,
         * so lets push back once. */
        libmatch_cursor_ungetch(&cursor);

        /* Validate the qualifier's opening */
        error_check_qualifier_header(cursor);

        /* What kind of qualifier are we handling? */
        if((qualifier = enumerate_qualifier(&cursor)) == QUALIFIER_UNKNOWN) {
            fprintf(stderr, "catalyst: unknown qualifier on line %i\n", cursor.line);
            exit(EXIT_FAILURE);
        }


    }

    return configuration;
}
