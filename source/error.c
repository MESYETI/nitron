#include "error.h"

const char* ErrorToString(Error err) {
	switch (err) {
		case N_ERROR_SUCCESS:               return "Success";
		case N_ERROR_FILE_NOT_FOUND:        return "File not found";
		case N_ERROR_BLOCK_DEVICE_REQUIRED: return "Block device required";
		case N_ERROR_CHAR_DEVICE_REQUIRED:  return "Character device required";
		case N_ERROR_FS_READ_ONLY:          return "Filesystem read only";
		case N_ERROR_FS_INACTIVE:           return "Filesystem inactive";
		case N_ERROR_DISK_INACTIVE:         return "Disk inactive";
		case N_ERROR_INTERNAL:              return "Internal error";
		case N_ERROR_OUT_OF_BOUNDS_DISK:    return "Out of bounds disk access";
		case N_ERROR_NOT_A_FILE:            return "Not a file";
		case N_ERROR_INVALID_DRIVE:         return "Invalid drive";
		default: return "???";
	}
}
