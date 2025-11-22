#ifndef N_ERROR_H
#define N_ERROR_H

#include "common.h"

enum {
	N_ERROR_SUCCESS               = 0,
	N_ERROR_FILE_NOT_FOUND        = 1,
	N_ERROR_BLOCK_DEVICE_REQUIRED = 2,
	N_ERROR_CHAR_DEVICE_REQUIRED  = 3,
	N_ERROR_FS_READ_ONLY          = 4,
	N_ERROR_FS_INACTIVE           = 5,
	N_ERROR_DISK_INACTIVE         = 6,
	N_ERROR_INTERNAL              = 7,
	N_ERROR_OUT_OF_BOUNDS_DISK    = 8,
	N_ERROR_NOT_A_FILE            = 9,
	N_ERROR_INVALID_DRIVE         = 10
};

typedef uint32_t Error;

const char* ErrorToString(Error err);

#endif
