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

#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "catalyst.h"

void handle_sigchild(int x) {
    int wstatus = 0;

    wait(&wstatus);
}

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

int main(int argc, char **argv) {
    struct Configuration configuration;

    signal(SIGCHLD, handle_sigchild);

    if(libpath_exists(CONFIGURATION_FILE) == 0) {
        fprintf(stderr, "catalyst: could not find configuration file '%s'\n", CONFIGURATION_FILE);
        exit(EXIT_FAILURE);
    }

    configuration = parse_configuration(CONFIGURATION_FILE);
    handle_jobs(configuration);

    free_configuration(configuration);

    return EXIT_SUCCESS;
}
