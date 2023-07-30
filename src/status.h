/**
 * @file status.h
 * @brief Defines system-level status codes.
 *
 * This file contains definitions for various system-level status codes that
 * can be returned by various parts of the operating system, such as the
 * kernel or device drivers.
 */

#ifndef STATUS_H
#define STATUS_H

/**
 * @def LAMEOS_OK
 * @brief A status code representing successful completion.
 */
#define LAMEOS_OK 0

/**
 * @def EIO
 * @brief A status code representing an input/output error.
 */
#define EIO 1

/**
 * @def EINVARG
 * @brief A status code representing an invalid argument error.
 */
#define EINVARG 2

/**
 * @def ENOMEM
 * @brief A status code representing an out of memory error.
 */
#define ENOMEM 3

#define EBADPATH 4

// Error Filesystem "Not Us"
#define EFSNOTUS 5

#define ERDONLY 6

#define EUNIMP 7


#endif
