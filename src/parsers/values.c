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
 * Functions for parsing different kinds of values that can appear as
 * values to keys inside of qualifiers.
*/

#include <math.h>
#include <ctype.h>

#include "../catalyst.h"

#define ADDCH_AND_CONCAT(character)             \
    if(cursor == READ_BUFFER_LENGTH) {          \
        cstring_concats(&new_cstring, buffer);  \
                                                \
        buffer[0] = '\0';                       \
    }

/*
 * @docgen: function
 * @brief: validate the syntax of an (unsigned) integer
 * @name: error_uinteger
 *
 * @description
 * @This functiion will verify that an unsigned integer does not contain
 * @any illegal characters. Unsigned numbers should only contain numeric
 * @characters, and scientific notation (xEy) is also forbidden.
 * @description
 *
 * @param cursor: the cursor to do error checking on
 * @type: struct LibmatchCursor
*/
void error_uinteger(struct LibmatchCursor cursor) {
    int character = 0;

    while((character = libmatch_cursor_getch(&cursor)) != '\n') {
        HANDLE_EOF(cursor);
    }
}

/*
 * @docgen: function
 * @brief: determine whether or not to continue reading a list
 * @name: continue_list
 *
 * @description
 * @This function will determine whether or not to continue reading a
 * @list of strings based off whether or not the current character,
 * @and the next character is a comma (,) and space ( ) respectively.
 * @description
 *
 * @param cursor: the cursor to use
 * @type: struct LibmatchCursor
 *
 * @return: 1 if list parsing should continue and 0 to stop
 * @type: int
*/
int continue_list(struct LibmatchCursor cursor) {
    int character_a = 0;
    int character_b = 0;

    character_a = libmatch_cursor_getch(&cursor);
    HANDLE_EOF(cursor);

    character_b = libmatch_cursor_getch(&cursor);
    HANDLE_EOF(cursor);

    if(character_a == ',' && character_b == ' ')
        return 1;

    if(character_a == '\n')
        return 0;
    
    fprintf(stderr, "catalyst: list on line %i expected ', ' before next element, got"
            " %c%c\n", cursor.line + 1, character_a, character_b);
    exit(EXIT_FAILURE);
}

unsigned int parse_uinteger(struct LibmatchCursor *cursor) {
    int index = 0;
    int length = 0;
    unsigned int number = 0;
    char number_ascii[NUMBER_LENGTH + 1] = "";

    liberror_is_null(parse_integer, cursor);
    error_uinteger(*cursor);

    /* Read the number and verify it is not too big */
    if((length = libmatch_read_until(cursor, number_ascii, NUMBER_LENGTH, "\n")) >= NUMBER_LENGTH) {
        fprintf(stderr, "catalyst: value in key/value pair on line %i is too big\n", cursor->line + 1);
        exit(EXIT_FAILURE);
    }

    /* Scan the number, and convert it into an actual number and verify it
     * along the way */
    for(index = 0; number_ascii[index] != '\0'; index++) {
        int base = length - index - 1;
        int digit = number_ascii[index];

        if(isdigit(digit) == 0) {
            fprintf(stderr, "catalyst: invalid character '%c' on line %i\n", digit, cursor->line + 1);
            exit(EXIT_FAILURE);
        }

        number = number + (digit - 48) * ((int) (pow(10, base)));
    }

    return number;
}

struct CString parse_string(struct LibmatchCursor *cursor) {
    int character = libmatch_cursor_getch(cursor);
    struct CString new_cstring = cstring_init("");

    HANDLE_EOF(*cursor);

    /* Value is missing an opening quote (0x22) */
    if(character != '"') {
        fprintf(stderr, "catalyst: expected start of value on line %i to be a '\"', got '%c'\n",
                cursor->line + 1, character);
        exit(EXIT_FAILURE);
    }

    /* Keep reading segments of the string until an unescaped " is
     * reached. */
    while(cursor->buffer[cursor->cursor] != '"') {
        int index = 0;
        int escaped = 0;
        char buffer[READ_BUFFER_LENGTH + 1] = "";

        /* Read until the buffer is full */
        while(index < READ_BUFFER_LENGTH && cursor->buffer[cursor->cursor] != '"') {
            character = libmatch_cursor_getch(cursor);

            HANDLE_EOF(*cursor);

            if(escaped == 0 && character == '\\') {
                escaped = 1;

                continue;
            }

            /* Interpret this character as the escaped form */
            if(escaped == 1) {
                switch(character) {
                    case 'n':
                        buffer[index] = '\n';
                        break;
                    case 'v':
                        buffer[index] = '\v';
                        break;
                    case 't':
                        buffer[index] = '\t';
                        break;
                }
            } else {
                buffer[index] = character;
            }

            index++;
            escaped = 0;
        }

        buffer[index] = '\0';
        cstring_concats(&new_cstring, buffer);
    }

    /* Go past the quote */
    libmatch_cursor_getch(cursor);

    return new_cstring;
}

struct CStrings *parse_string_list(struct LibmatchCursor *cursor) {
    struct CStrings *cstring_list = carray_init(cstring_list, CSTRING);

    while(cursor->cursor < cursor->length) {
        struct CString next_string = parse_string(cursor);

        carray_append(cstring_list, next_string, CSTRING);

        if(continue_list(*cursor) == 0)
            break;

        /* Go past the ', ' */
        libmatch_cursor_getch(cursor);
        libmatch_cursor_getch(cursor);
    }

    return cstring_list;
}








