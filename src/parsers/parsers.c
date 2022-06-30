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
    ASSERT_NEXT_CHARACTER(&cursor, '\n');
}

/*
 * @docgen: function
 * @brief: validate a line in a qualifier block
 * @name: error_check_qualifier_line
 *
 * @description
 * @This function will take the current position of the cursor inside
 * @of a qualifier block, and validate its general format to produce
 * @errors if it is incorrectly written.
 * @description
 *
 * @notes
 * @This function does not do any validation on the actualy data assigned
 * @to the value of the key. When it comes to the values of the key, all it
 * @does is check that SOMETHING is there.
 * @notes
 *
 * @param cursor: the cursor to read from
 * @type: struct LibmatchCursor
*/
void error_check_qualifier_line(struct LibmatchCursor cursor) {
    int index = 0;
    int character = -1;

    /* Start of the line must have 4 pixels at the start */
    while(index < 4) {
        character = libmatch_cursor_getch(&cursor);

        HANDLE_EOF(cursor);

        if(character == ' ') {
            index++;

            continue;
        }

        fprintf(stderr, "catalyst: body line of qualifier at line %i must have 4 spaces at the start\n", cursor.line + 1);
        exit(EXIT_FAILURE);
    }

    character = libmatch_cursor_getch(&cursor);
    HANDLE_EOF(cursor);

    /* First character after a space must be in the character class
     * [A-Za-z_]. */
    if(strchr(LIBMATCH_ALPHA "_", character) == NULL) {
       fprintf(stderr, "catalyst: expected alphabetical character or underscore"
               " after initial 4 spaces on line %i, got %c\n", cursor.line + 1,
               character);
       exit(EXIT_FAILURE);
    }

    /* All characters after expression ' {4}[a-zA-Z_]' must fit in
     * the character class [a-zA-Z0-9_] until a colon is reached. */
    while((character = libmatch_cursor_getch(&cursor)) != ':') {
        HANDLE_EOF(cursor);

        if(strchr(LIBMATCH_ALPHANUM "_", character) != NULL)
            continue;

        fprintf(stderr, "catalyst: expected alphanumerical character or underscore"
               " after first character after initial 4 spaces on line %i, got %c\n", cursor.line + 1,
               character);
        exit(EXIT_FAILURE);
    }

    /* Everything else after the colon must follow the expression:
     * ' [^\s]' */
    ASSERT_NEXT_CHARACTER(&cursor, ' ');

    character = libmatch_cursor_getch(&cursor);
    HANDLE_EOF(cursor);

    /* Not an invalid character */
    if(strchr(LIBMATCH_WHITESPACE, character) == NULL)
        return;

    fprintf(stderr, "catalyst: expected non-empty value to key on line %i\n",
            cursor.line + 1);
    exit(EXIT_FAILURE);
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

/*
 * @docgen: function
 * @brief: enumerate the name of a job's key
 * @name: enumerate_job_key
 *
 * @description
 * @This function will, with the cursor on the first character of
 * @the name of the key, return an integer describing the name of
 * @the key.
 * @description
 *
 * @notes
 * @This function expects to be in the body of a JOB, so do not
 * @expect it to work with a testcase body.
 * @notes
 *
 * @param cursor: the cursor to enumerate through
 * @type: struct LibmatchCursor *
 *
 * @return: an integer describing the key, or QUALIFIER_UNKNOWN
*/
int enumerate_job_key(struct LibmatchCursor *cursor) {
    int written = 0;
    char job_key_name[JOB_KEY_NAME_LENGTH + 1] = "";

    written = libmatch_read_until(cursor, job_key_name, JOB_KEY_NAME_LENGTH, ":");

    /* Make sure it did not overflow. */
    if(written >= JOB_KEY_NAME_LENGTH) {
        fprintf(stderr, "catalyst: qualifier job key name on line %i too long\n", cursor->line + 1);
        exit(EXIT_FAILURE);
    }

    if(strcmp(job_key_name, "name") == 0)
        return QUALIFIER_JOB_NAME;

    if(strcmp(job_key_name, "make") == 0)
        return QUALIFIER_JOB_MAKE;

    if(strcmp(job_key_name, "arguments") == 0)
        return QUALIFIER_JOB_ARGUMENTS;

    return QUALIFIER_UNKNOWN;
}

/*
 * @docgen: function
 * @brief: determine if a cursor is at the end of a qualifier
 * @name: end_of_qualifier
 *
 * @description
 * @This function will determine whether or not the cursor, given
 * @its current position, is at the end of a block.
 * @description
 *
 * @notes
 * @This function recieves a copy of the cursor, and so it will not
 * @modify the cursor of the caller.
 * @notes
 *
 * @param cursor: the cursor to check
 * @type: struct LibmatchCursor
 *
 * @return: 1 if its at the end of a qualifier, and 0 if its not
 * @type: int
*/
int end_of_qualifier(struct LibmatchCursor cursor) {
    int character = -1;

    character = libmatch_cursor_getch(&cursor);
    HANDLE_EOF(cursor);

    if(character == '}') {
        character = libmatch_cursor_getch(&cursor);
        HANDLE_EOF(cursor);

        if(character == '\n')
            return 1;
    }

    return 0;
}

struct LibmatchCursor open_cursor_stream(const char *path) {
    FILE *stream = fopen(path, "r");
    struct LibmatchCursor cursor;

    INIT_VARIABLE(cursor);
    cursor = libmatch_cursor_from_stream(stream);
    fclose(stream);

    return cursor;
}




/*
 * PARSING LOGIC
*/

struct Job parse_job(struct LibmatchCursor *cursor, struct ParserState *state) {
    struct Job new_job;

    /* Prepare for parsing */
    INIT_VARIABLE(new_job);
    cstring_reset(&state->line);

    /* We should be 'in' the body of the job, and so on the first
     * line of it. The actual format should have the closing brace
     * strictly at the start of the line, and be the only character
     * except for the new line on it. So we can detect when to stop
     * by reading a full line, and checking if it is "}\n". Otherwise,
     * perform job line error checks.
    */
    while(end_of_qualifier(*cursor) == 0) {
        int key = 0;

        error_check_qualifier_line(*cursor);

        /* 4 initial spaces on each line */
        libmatch_cursor_getch(cursor);
        libmatch_cursor_getch(cursor);
        libmatch_cursor_getch(cursor);
        libmatch_cursor_getch(cursor);

        /* What kind of job key are we handling? */
        if((key = enumerate_job_key(cursor)) == QUALIFIER_UNKNOWN) {
            fprintf(stderr, "catalyst: unknown job qualifier key on line %i\n", cursor->line + 1);
            exit(EXIT_FAILURE);
        }

        /* Go pass the space */
        libmatch_cursor_getch(cursor);
        parse_uinteger(cursor);

    }

    return new_job;
}

struct Configuration parse_configuration(const char *path) {
    struct ParserState state;
    struct LibmatchCursor cursor;
    struct Configuration configuration;

    liberror_is_null(parse_configuration, path);

    INIT_VARIABLE(state);
    INIT_VARIABLE(cursor);
    INIT_VARIABLE(configuration);

    /* Initialize stuff */
    cursor = open_cursor_stream(path);
    state.line = cstring_init("");

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
            fprintf(stderr, "catalyst: unknown qualifier on line %i\n", cursor.line + 1);
            exit(EXIT_FAILURE);
        }

        /* Jump to next line */
        libmatch_next_line(&cursor);

        /* Decide which block to parse. */
        parse_job(&cursor, &state);
    }

    return configuration;
}
