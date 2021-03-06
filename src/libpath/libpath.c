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
 * Implementations of the libpath functions.
*/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "libpath.h"
#include "lp_inter.h"

int libpath_join_path(char *buffer, int length, ...) {
    int written = 0;
    va_list path_segment;
    char *segment = NULL;

    INIT_VARIABLE(path_segment);
    va_start(path_segment, length);

    segment = va_arg(path_segment, char *);

    while(segment != NULL) {
        int cursor = 0;

        /* Write the path segment to the buffer */
        while(written < length && segment[cursor] != '\0') {
            buffer[written] = segment[cursor];
            cursor++;
            written++;
        }

        cursor = 0;
        segment = va_arg(path_segment, char *);

        if(segment == NULL)
            break;


        /* Write the path separator if the next one is not NULL */
        while(written < length && LIBPATH_SEPARATOR[cursor]) {
            buffer[written] = LIBPATH_SEPARATOR[cursor];
            cursor++;
            written++;
        }
    }

    buffer[written] = '\0';
    
    return written;
}

int libpath_exists(const char *path) {
    struct stat stat_buffer;

    liberror_is_null(libpath_exists, path);

    if(stat(path, &stat_buffer) == -1)
        return 0;

    return 1;
}

int libpath_rmdir(const char *path) {
    return rmdir(path);
}

int libpath_mkdir(const char *path, int mode) {
#if defined(_MSDOS)
    return mkdir(path);
#endif

#if defined(__WATCOMC__)
    return mkdir(path);
#endif

#if defined(__unix__)
    return mkdir(path, mode);
#endif
}

/*
 * GLOBBING LOGIC
*/

/*
 * Honestly, this function is likely to be the most complex function in 
 * libpath, so it deserves a lot of explanation. This function determines 
 * whether or not a given path will 'match' a glob. Currently, this function
 * supports the following syntax:
 *
 * * - Match an arbitrary number of characters until the character after it
 *     is found.
 *
 *     EXAMPLE:
 *        *.txt - Match any character until a period
 *        *.*   - Match any character until a period, then any character until the
 *                NUL byte.
 *        foo*  - Match 'foo' then stop at the NUL byte
*/
static int matches_glob(const char *name, const char *pattern) {
    int name_cursor = 0;
    int pattern_cursor = 0;

    while(name[name_cursor] != '\0') {
        int stop_char = '\0';

        /* Match characters in the pattern until a wildcard
         * character is found. */
        while(pattern[pattern_cursor] != '*') {
            if(pattern[pattern_cursor] == '\0' || name[name_cursor] == '\0') {
                /* Pattern exhausted before the name */
                if(pattern[pattern_cursor] == '\0' && name[name_cursor] != '\0')
                    return 0;

                break;
            }

#if defined(_MSDOS)
            if(toupper(pattern[pattern_cursor]) == toupper(name[name_cursor])) {
                name_cursor++;
                pattern_cursor++;

                continue;
            }
#else
            if(pattern[pattern_cursor] == name[name_cursor]) {
                name_cursor++;
                pattern_cursor++;

                continue;
            }
#endif

            /* Match not valid! */
            return 0;
        }

        /* Now, unless the glob is a flat match, e.g ('foo' and not 'foo*'),
         * then pattern[pattern_cursor] should be an asterisk, which implies
         * a wild card. */
        if(pattern[pattern_cursor] == '*') {
            stop_char = pattern[pattern_cursor + 1]; /* Can be a NUL byte */
            pattern_cursor++;
        }

        /* Keep matching until the stop_char is found. In situations
         * where this is a flat match, this will just immediately stop,
         * as name[name_cursor] will be on the NUL byte. This section
         * will essentially just exhaust the wildcard.
        */
#if defined(_MSDOS)
        while(toupper(name[name_cursor]) != toupper(stop_char) && toupper(name[name_cursor]) != '\0')
            name_cursor++;
#else
        while(name[name_cursor] != stop_char && name[name_cursor] != '\0')
            name_cursor++;
#endif

        /* Name exhausted before the path */
#if defined(_MSDOS)
        if(toupper(name[name_cursor]) == '\0' && toupper(pattern[pattern_cursor]) != '\0')
            return 0;
#else
        if(name[name_cursor] == '\0' && pattern[pattern_cursor] != '\0')
            return 0;
#endif

    }

    return 1;
}

#if defined(__unix__)
struct LibpathFiles libpath_glob(const char *path, const char *pattern) {
    DIR *directory = NULL;
    struct dirent *entry = NULL;
    struct LibpathFiles globbed_files;
 
    INIT_VARIABLE(globbed_files);

    /* Rather than use the base carray initialization logic, we do
     * our own initialization because carray has no way to initialize
     * a stack structure but with a heap contents field. */
    globbed_files.length = 0;
    globbed_files.capacity = 5;
    globbed_files.contents = malloc(sizeof(struct LibpathFile) * 5);

    directory = opendir(path);

    /* Iterate through the contents of this directory. */
    while((entry = readdir(directory)) != NULL) {
        struct LibpathFile new_path;

        /* No thanks */
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        /* This path does not match the glob pattern-- ignore it */
        if(matches_glob(entry->d_name, pattern) == 0)
            continue;

        if(libpath_join_path(new_path.path, LIBPATH_GLOB_PATH_LENGTH, path,
                             entry->d_name, NULL) >= LIBPATH_GLOB_PATH_LENGTH) {
            liberror_unhandled(libpath_glob);
        }

        carray_append(&globbed_files, new_path, FILE);
    }

    closedir(directory);

    return globbed_files;
}
#endif

#if defined(_MSDOS)
struct LibpathFiles libpath_glob(const char *path, const char *pattern) {
    int status = -1;
    struct _find_t node;
    struct LibpathFiles globbed_files;
    char glob_path[LIBPATH_GLOB_PATH_LENGTH + 1] = "";
 
    INIT_VARIABLE(node);
    INIT_VARIABLE(globbed_files);

    /* Rather than use the base carray initialization logic, we do
     * our own initialization because carray has no way to initialize
     * a stack structure but with a heap contents field. */
    globbed_files.length = 0;
    globbed_files.capacity = 5;
    globbed_files.contents = malloc(sizeof(struct LibpathFile) * 5);

    /* Build the path to the glob. */
    if(libpath_join_path(glob_path, LIBPATH_GLOB_PATH_LENGTH, path, "*.*",
                         NULL) >= LIBPATH_GLOB_PATH_LENGTH) {
        fprintf(stderr, "libpath_glob: could not glob path '%s': glob path full\n", glob_path);
        exit(EXIT_FAILURE);
    }

    /* Begin node iteration */
    status = _dos_findfirst(glob_path, _A_NORMAL | _A_SUBDIR, &node);

    /* Iterate through the contents of this node. */
    while(status == 0) {
        struct LibpathFile new_path;

        /* No thanks */
        if(strcmp(node.name, ".") == 0 || strcmp(node.name, "..") == 0) {
            status = _dos_findnext(&node);

            continue;
        }

        /* This path does not match the glob pattern-- ignore it */
        if(matches_glob(node.name, pattern) == 0)  {
            status = _dos_findnext(&node);

            continue;
        }

        if(libpath_join_path(new_path.path, LIBPATH_GLOB_PATH_LENGTH, path,
                             node.name, NULL) >= LIBPATH_GLOB_PATH_LENGTH) {
            liberror_unhandled(libpath_glob);
        }

        carray_append(&globbed_files, new_path, FILE);

        status = _dos_findnext(&node);
    }

    /* Handle errors */
    if(status == -1) {
        fprintf(stderr, "libpath_glob: failed to glob path '%s' (%s)\n", glob_path,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    return globbed_files;
}
#endif

void libpath_free_glob(struct LibpathFiles files) {
    free(files.contents);
}

/*
 * ITERATION LOGIC
*/
struct LibpathDirState libpath_directory_iter_start(const char *path) {
    struct LibpathDirState new_state;

    INIT_VARIABLE(new_state);

    new_state.status = 1;

    /* Directory opening can fail */
    if((new_state.directory = opendir(path)) == NULL) {
        fprintf(stderr, "libpath_directory_iter: failed to open directory '%s' (%s)\n",
                path, strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* No nodes in the directory */
    if((new_state.entry = readdir(new_state.directory)) == NULL)
        new_state.status = 0;

    /* Initialize directory state node */
    if(libpath_join_path(new_state.path, LIBPATH_MAX_PATH, new_state.entry->d_name, NULL) >= LIBPATH_MAX_PATH) {
        fprintf(stderr, "libpath_directory_iter: directory path '%s%s%s' cannot fit in"
                "path buffer\n", path, LIBPATH_SEPARATOR, new_state.entry->d_name);
        exit(EXIT_FAILURE);
    }

    return new_state;
}
