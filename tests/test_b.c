#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>

#include "common.h"

int main(int argc, char **argv) {
    printf("%s", "AAAAAAAAAAA\n");

    abort();

    return 0;
}
