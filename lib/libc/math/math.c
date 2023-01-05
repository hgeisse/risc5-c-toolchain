#include <math.h>
#include <stdint.h>

double fabs(double x)
{
    /* NOTE: double on RISC5 is 32bit */
    /* NOTE: the union is used to satisfy C aliasing rules */
    union {double f; uint32_t i;} u;
    u.f = x;
    u.i &= 0x7FFFFFFF;  /* clear sign bit */
    return u.f;
}
