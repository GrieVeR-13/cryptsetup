#ifndef EDS3_URAIO_CONFIG_H
#define EDS3_URAIO_CONFIG_H

#include <cutil/uraio.h>

#define open uraio_open
#define close uraio_close
#define lseek uraio_lseek
#define lseek64 uraio_lseek64
#define read uraio_read
#define write uraio_write
#define fsync uraio_fsync
#define fstat uraio_fstat
#define stat(PATH, STAT) uraio_stat(PATH, STAT)
#define flock uraio_flock
#define fpathconf uraio_fpathconf

#endif //EDS3_URAIO_CONFIG_H
