#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "common.h"

int main(int argc, char **argv) {
    sleep(2);
    abort();

    return 0;
}
