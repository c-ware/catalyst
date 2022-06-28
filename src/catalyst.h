#ifndef CWARE_CATALYST_H
#define CWARE_CATALYST_H

#include "carray/carray.h"
#include "cstring/cstring.h"
#include "libpath/libpath.h"
#include "libmatch/libmatch.h"

#include "parsers/parsers.h"

/* Configuration */
#define CONFIGURATION_FILE  ".catalyst"

/* Useful macros */
#define INIT_VARIABLE(v) \
    memset(&(v), 0, sizeof((v)))

#endif
