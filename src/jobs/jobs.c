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
 * Functions for handling the running of jobs and testcases.
*/

#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"
#include "../catalyst.h"

void free_configuration(struct Configuration configuration) {
    int index = 0;

    /* Release the jobs */
    for(index = 0; index < carray_length(configuration.jobs); index++) {
        int array_index = 0;
        struct Job job = configuration.jobs->contents[index];

        cstring_free(job.name);
        cstring_free(job.make_path);

        for(array_index = 0; array_index < carray_length(job.make_arguments); array_index++) {
            cstring_free(job.make_arguments->contents[array_index]);
        }

        free(job.make_arguments->contents);
        free(job.make_arguments);
    }

    free(configuration.jobs->contents);
    free(configuration.jobs);

    /* Release the test cases */
    for(index = 0; index < carray_length(configuration.testcases); index++) {
        int array_index = 0;
        struct Testcase testcase = configuration.testcases->contents[index];

        cstring_free(testcase.path);
        cstring_free(testcase.input);
        cstring_free(testcase.output);

        for(array_index = 0; array_index < carray_length(testcase.argv); array_index++) {
            cstring_free(testcase.argv->contents[array_index]);
        }

        free(testcase.argv->contents);
        free(testcase.argv);
    }

    free(configuration.testcases->contents);
    free(configuration.testcases);
}

/*
 * @docgen: function
 * @brief: verify the existence of all testcase binaries
 * @name: verify_testcase_validity
 *
 * @description
 * @For each test case, verify that the binary that is intended
 * @to be executed actually exists.
 * @description
 *
 * @param configuration: the configuration containing the testcases
 * @type: struct Configuration
*/
void verify_testcase_validity(struct Configuration configuration) {
    int index = 0;
    struct CString path_string = cstring_init("");

    for(index = 0; index < carray_length(configuration.testcases); index++) {
        struct Testcase testcase = configuration.testcases->contents[index];

        /* Make the path. Reset it first, though. */
        cstring_reset(&path_string);
        cstring_concats(&path_string, TESTS_DIRECTORY);
        cstring_concats(&path_string, LIBPATH_SEPARATOR);
        cstring_concat(&path_string, testcase.path);

        /* Path exists-- move on. */
        if(libpath_exists(path_string.contents) == 1)
            continue;

        fprintf(stderr, "catalys: testcase file '%s' does not exist\n", path_string.contents);
        exit(EXIT_FAILURE);
    }

    cstring_free(path_string);
}

void test_runner(struct Testcase testcase, struct PipePair pair) {
    int pid = 0;
    int index = 0;
    int exit_code = 0;

    /* Disable signal handling in the test runner and the test! (in the test
     * until the process image is replaced, at least.) */
    signal(SIGCHLD, SIG_DFL);

    /* Prepare the child process (will become the test). Should be noted that
     * all allocations under this block will not need to be released due
     * to the process image replacement. */
    if((pid = fork()) == 0) {
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

        /* Lock and load */
        execv(test_path.contents, argv);
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

void handle_jobs(struct Configuration configuration) {
    int index = 0;
    int responses = 0;
    struct PipePairs *pipes = NULL;
    struct Pollfds *descriptors = NULL;

    pipes = carray_init(pipes, PIPE_PAIR);
    descriptors = carray_init(descriptors, POLLFD);
    
    verify_testcase_validity(configuration);

    /* Run all tests (note that this does actually do any handling
     * of the communication between the processes. It only readies
     * the infrastructure to handle it. */
    for(index = 0; index < carray_length(configuration.testcases); index++) {
        int fork_pipes[2];
        struct PipePair pair;
        struct pollfd poll_segment;

        /* Setup communication between root process and test runner */
        pipe(fork_pipes);
        pair.read = fork_pipes[0];
        pair.write = fork_pipes[1];

        /* Root process needs to wait for confirmation that the test
         * has completed. Test runners will notify the root process
         * of the exit code, the file that was executed, and a string
         * containing the reason it failed. */
        poll_segment.fd = pair.read;
        poll_segment.events = POLLIN;

        /* Let the test runner do its thing. */
        switch(fork()) {
            case 0: 
                /* Test runner has access to a bunch of stuff due to
                 * the need to release heap memory, and access IPC
                 * interfaces. */
                test_runner(configuration.testcases->contents[index], pair);
                
                /* Cleanup the cloned memory */
                free_configuration(configuration);
                carray_free(pipes, PIPE_PAIR);
                carray_free(descriptors, POLLFD);

                exit(EXIT_SUCCESS);
            case -1:
                liberror_failure(handle_jobs, fork);

                exit(EXIT_FAILURE);
        }

        carray_append(pipes, pair, PIPE_PAIR);
        carray_append(descriptors, poll_segment, POLLFD);
    }

    /* Wait until all processes have completed their jobs */
    while(responses != carray_length(descriptors)) {
        responses = poll(descriptors->contents, carray_length(descriptors), PROCESS_CHECK_TIMEOUT);

        if(responses > 0) 
            continue;

        /* Interruption- This is an unavoidable error at times, so
         * keep going. */
        if(errno == EINTR) {
            errno = 0;

            continue;
        }

        /* Stop-- error time  */
        liberror_failure(handle_jobs, poll);
    }

    /* Read all responses */
    for(index = 0; index < carray_length(pipes); index++) {
        char response[PROCESS_RESPONSE_LENGTH + 1] = "";

        /* Read the response */
        INIT_VARIABLE(response);
        read(pipes->contents[index].read, response, PROCESS_RESPONSE_LENGTH);

        printf("%s\n", response);
    }

    /* File descriptors are closed in the pipe array releasing function */
    carray_free(pipes, PIPE_PAIR);
    carray_free(descriptors, POLLFD);
}
