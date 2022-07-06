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
 * This file contains logic for handling the building and running of jobs.
 * When a job is built, the test running logic begins. Test running logic
 * can be found in src/testing.
*/

#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"
#include "../catalyst.h"
#include "../common/common.h"
#include "../parsers/parsers.h"

void start_test_runners(struct PipePairs *pipes, struct Pollfds *descriptors,
                        struct Configuration configuration) {
    int index = 0;

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
                handle_testcase(configuration.testcases->contents[index], pair);
                
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
}

void wait_for_completion(struct Pollfds descriptors) {
    int responses = 0;

    while(responses != carray_length(&descriptors)) {
        responses = poll(descriptors.contents, carray_length(&descriptors), PROCESS_CHECK_TIMEOUT);

        if(responses > 0) 
            continue;

        /* Interruption- This is an unavoidable error at times, so
         * keep going. */
        if(errno == EINTR) {
            errno = 0;

            continue;
        }

        /* Stop-- error time  */
        liberror_failure(wait_for_completion, poll);
    }
}

void process_responses(struct PipePairs pipes) {
    int index = 0;

    for(index = 0; index < carray_length(&pipes); index++) {
        char response[PROCESS_RESPONSE_LENGTH + 1] = "";

        /* Read the response */
        INIT_VARIABLE(response);
        printf("...\n");
        read(pipes.contents[index].read, response, PROCESS_RESPONSE_LENGTH);
    }
}

void handle_jobs(struct Configuration configuration) {
    struct PipePairs *pipes = NULL;
    struct Pollfds *descriptors = NULL;

    pipes = carray_init(pipes, PIPE_PAIR);
    descriptors = carray_init(descriptors, POLLFD);
    
    /* Spawn test runners to run their tests, wait for the tests to finish,
     * and read the responses from the test runners. */
    start_test_runners(pipes, descriptors, configuration);
    wait_for_completion(*descriptors);
    process_responses(*pipes);

    /* File descriptors are closed in the pipe array releasing function */
    carray_free(pipes, PIPE_PAIR);
    carray_free(descriptors, POLLFD);
}
