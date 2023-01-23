/*
 * stdint.h -- known width integer types
 */

#ifndef __STDINT_H
#define __STDINT_H

/*********************************************
 * NOTE: This is a not a full implementation 
 *       of stdint.h, just the bare minimum
 *********************************************/

#include <stddef.h>

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

#ifndef SIZE_MAX
    #define SIZE_MAX    ( ( size_t ) -1 )
#endif

#endif /* __STDINT_H */
