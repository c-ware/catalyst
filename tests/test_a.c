#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "common.h"

int main(int argc, char **argv) {
    int index = 0;
    char buffer[4096 + 1] = {0};

    fread(buffer, 1, 4096, stdin);
    printf("Read text: '%s'\n", buffer);

    return 2;
}
