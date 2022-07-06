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
 * This file has routines for executing a testcase.
*/

#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "testing.h"
#include "../catalyst.h"
#include "../jobs/jobs.h"
#include "../parsers/parsers.h"

void handle_testcase(struct Testcase testcase, struct PipePair pair) {
    int pid = 0;
    int index = 0;
    int exit_code = 0;
    int parent_to_child[2] = {0, 0};
    int child_to_parent[2] = {0, 0};
    struct CString string = cstring_init("");

    /* Disable signal handling in the test runner and the test! (in the test
     * until the process image is replaced, at least.) */
    signal(SIGCHLD, SIG_DFL);

    /* Only prepare parent_to_child if, and only if there is input to
     * that the test should expect, please. */
    if(testcase.input.contents != NULL) {
        pipe(parent_to_child);
        pipe(child_to_parent);
    }

    /* Prepare the child process (will become the test). Should be noted that
     * all allocations under this block will not need to be released due
     * to the process image replacement. */
    if((pid = fork()) == 0) {
        int flags = 0;
        char **argv = NULL;
        struct CString test_path = cstring_init(TESTS_DIRECTORY);

        /* Build the path to the testcase */
        cstring_concats(&test_path, LIBPATH_SEPARATOR);
        cstring_concat(&test_path, testcase.path);

        /* Prepare the argv (+1 for NULL at the end of the array, and
         * inserting at 0 for the path to the test */
        carray_insert(testcase.argv, 0, test_path, CSTRING);
        argv = malloc((carray_length(testcase.argv) + 1) * sizeof(char *));

        /* Extract pointers to all the contents of each argument */
        for(index = 0; index < carray_length(testcase.argv); index++) {
            argv[index] = testcase.argv->contents[index].contents;
        }

        /* NULL so that execv can work on the array */
        argv[index] = NULL;

        /* Oh boy. This section has to be the single most confusing part in
         * this entire program. This might take a little bit to explain.
         *
         * So to start, let's remember the purpose of this function.
         * This function, which is spawned by handle_jobs, serves the
         * purpose of running an individual test.
         *
         * Part of running a test involves, among other things, writing
         * input into the test's stdin to make sure it produces the expected
         * output. This might seem simple at first, and it actually kind of
         * is *initially.* In order to communicate with the child process,
         * we must first steup a method of communication. The standard UNIX
         * approach is to use parent_to_child for this kind of logic, and that is what
         * we do.
         *
         * Now, as I am sure you know, the pipe() function will create a write,
         * and read end of the pipe. We want the subprocess to read from the
         * end of the pipe, so that we can pipe data through it-- effectively
         * making a 'new' standard input stream so to speak.
         *
         * To do this, we must first understand how UNIX internally handles
         * file descriptors. Internally, there is a table of allocated file
         * descriptors for a process that, as new files are opened, get
         * assigned to whatever file descriptor is closest to the start of
         * the list.
         *
         * Because of this behavior, this means that when you close a file
         * descriptor, assuming all file descriptor slots below it are being
         * used, it can then be 'refilled' with a new file descriptor. We can
         * exploit this behavior to essentially assign a new file to the stdn.
         *
         * Under POSIX, the standard streams have 'reserved' file descriptors
         * in the sense that the libc should respect that STDIN_FILENO (which
         * is 0 under POSIX) will always be treated as the file descriptor of
         * the stdin stream.
         * 
         * This means, that if we close the file descriptor STDIN_FILENO, which
         * is quite literally the first expected file descriptor, it will
         * become available to be overwritten. This will allow us to 'change'
         * the file mapped to file descriptor 0 by duplicating the read end of
         * the pipe with dup(2), which will create a NEW file descriptor at the
         * closest unused slot to the start of the table, which is mapped to
         * the same file as the read end of the pipe. We then essentially do
         * the same thing as above, but we redirect the stdout of our program
         * to another pair of pipes so that we do not attempt to the subprocess's
         * read pipe.
         *
         * Now that is a lot of text to explain two lines of code, but trust me
         * it gets worse. We now enter the area of having to use fcntl to allow
         * for non-blocking I/O.
         *
         * Without setting O_NONBLOCK on the stdin's new file descriptor, calls
         * like fgetc will continue to infinitely block when there is no more
         * data there, not sure why. To replicate Bash not spawning a pipe when
         * there is no  (for the sake of people's expectations of what will
         * happen when trying to fgetc in a test with no stdin expected), a pipe
         * will not be created unless input is expected. We also do this because
         * it might be better to just have it block than silently have fgetc
         * return -1, not sure, we will see.
         *
         * Without setting O_NONBLOCK on the stdin's new file descriptor, calls
         * like fgetc will continue to infinitely block when there is no more
         * data there, not sure why. Something this code originally did was it
         * would open a pipe to stdin regardless of whether or not there was
         * actually data to read.
         *
         * This both causes a waste of file descriptor space, and can
         * potentially cause bugs in existing programs which rely on having an
         * infinite pause when there is no pipe setup. The infinite pause is,
         * as far as I can tell, caused by the child process inheriting the
         * stdin of the parent process, and not modifying it to use a pipe
         * due to not having any actual data to read. When stdin is still
         * connected to a terminal, then as far as I can tell, read will have
         * special reading behavior for terminals, and will just pause forever.
        */
        if(testcase.input.contents != NULL) {
            close(STDIN_FILENO);
            dup(parent_to_child[0]);

            close(STDOUT_FILENO);
            dup(child_to_parent[1]);

            /* Close unneeded pipe ends for this fork */
            close(parent_to_child[1]);
            close(child_to_parent[0]);

            flags = fcntl(parent_to_child[0], F_GETFL, 0);
            flags |= O_NONBLOCK;
            fcntl(parent_to_child[0], F_SETFL, flags);
        }

        execv(test_path.contents, argv);
    }

    /* Close unnecessary ends of the pipe for the parent process, as the
     * child process has already inherited its own. */
    close(parent_to_child[0]);
    close(child_to_parent[1]);

    /* Write the stdin to the pipe if there is any to write */
    if(testcase.input.contents != NULL) {
        write(parent_to_child[1], testcase.input.contents, testcase.input.length);
    }
    
    /* Wait for a timeout */
    if(testcase.timeout != 0) {
        /* WIP */

        return; 
    }

    /* Wait for the child process to quit, and extract the exit code */
    wait(&exit_code);
    printf("Process %i exited with %i\n", pid, WEXITSTATUS(exit_code));
}
