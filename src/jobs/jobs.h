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

#ifndef CWARE_CATALYST_JOBS_H
#define CWARE_CATALYST_JOBS_H

/*
 * @docgen: structure
 * @brief: pair of pipes for reading and writing
 * @name: PipePair
 *
 * @field read: the pipe to read from
 * @type: int
 *
 * @field write: the pipe to write to
 * @type: int
*/
struct PipePair {
    int read;
    int write;
};

/*
 * @docgen: structure
 * @brief: array of pipes
 * @name: PipePairs
 *
 * @field length: the length of the array
 * @type: int
 *
 * @field capacity: the capacity of the array
 * @type: int
 *
 * @field contents: the pipes in the array
 * @type: struct PipePair
*/
struct PipePairs {
    int length;
    int capacity;
    struct PipePair *contents;
};

/*
 * @docgen: structure
 * @brief: an array of integers
 * @name: IntArray
 *
 * @field length: the length of the array
 * @type: int
 *
 * @field capacity: the capacity of the array
 * @type: int
 *
 * @field contents: the pollfds in the array
 * @type: struct pollfd *
*/
struct Pollfds {
    int length;
    int capacity;
    struct pollfd *contents;
};

/* Data structure properties */
#define PIPE_PAIR_TYPE  struct PipePair
#define PIPE_PAIR_HEAP  1
#define PIPE_PAIR_FREE(value) \
    close((value).read);      \
    close((value).write)

#define POLLFD_TYPE  struct pollfd
#define POLLFD_HEAP  1
#define POLLFD_FREE(value)


#endif
