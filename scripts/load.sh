#!/bin/sh
# Load Makefiles, and documentation.

makegen project unix --binary catalyst --main src/main.c \
                     --ldlibs '\-lm' > Makefile

makegen project unix --binary catalyst --main src/main.c \
                     --ldlibs '\-lm' --cflags '\-Wall -Wextra -Wpedantic -Wshadow -ansi -g -Wno-unused-parameter -Wno-type-limits -Wno-sign-compare' > Makefile.dev

# Make gitignore
m4 template/.gitignore > .gitignore
