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

/*
 * @docgen: function
 * @brief: validate the syntax of an (unsigned) integer
 * @name: error_uinteger
 *
 * @description
 * @This functill will verify that an unsigned integer does not contain
 * @any illegal characters. Unsigned numbers should only contain numeric
 * @characters, and scientific notation (xEy) is also forbidden.
 * @description
 *
 * @param cursor: the cursor to do error checking on
 * @type: struct LibmatchCursor
*/
static void error_uinteger(struct LibmatchCursor cursor) {
    int character = 0;

    while((character = libmatch_cursor_getch(&cursor)) != '\n') {
        HANDLE_EOF(cursor);
    }
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
