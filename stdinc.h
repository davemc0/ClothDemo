/*****************************************************************************

  stdinc.h

  A collection of macros and #defines

  #defines:
    true, false, M_PI
    AND, OR, NOT       - logical (not bitwise) operators
    String             - 81 chars
    Bool, Byte         - unsigned char
    FUDGE              - .0001, for zero-testing

  macros:
    MIN, MAX           - min and max of two values
    ABS                - absolute value
    TOGGLE_BOOL(x)     - flips the boolean value of x
    TEST_BIT(x,b)      - checks if bit 'b' of x is set
    SET_BIT(x,b)       - sets bit 'b' of x
    CLEAR_BIT(x,b)     - clears bit 'b' of x
    TOGGLE_BIT(x,b)    - toggles bit 'b' of x
    flushout           - short for fflush(stdout)
    flusherr           - short for fflush(stderr)
    randf              - returns a random value between 0 and 1
    SIGN(x)            - returns 1 for positive, -1 for negative
    DEG2RAD, RAD2DEG   - radian-degree conversion
    CLAMP(x,lo,hi)     - clamps x to the interval (lo,hi)
    IN_BOUNDS(x,lo,hi) - tests whether x is between lo and hi
    PT_IN_BOX          - tests two values (x and y) against two intervals
    CHECK_PROPER_SIDE  - checks if value lies on proper side (above or
                         below) another value
    SWAP2(a,b,t)       - swaps a and b, using a temp variable t
    VEC3_TO_ARRAY      - Copies three values in one array into another array

  data structs:
    RGBc               - r, g, and b in Byte format

  ----------------------------------------------------------------------

  - 1997 Paul Rademacher (rademach@cs.unc.edu)

******************************************************************************/

#ifndef _STDINC_H_
#define _STDINC_H_

#include <math.h>
#include <stdio.h>

#ifndef AND
#define AND &&
#define OR ||
#define NOT !
#endif

#ifndef true
#define true 1
#define false 0
#endif

#ifndef Bool
typedef char Bool;
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : (-(a)))
#endif

/********* TOGGLE_BOOL(boolean) : toggles values of 'boolean' ******/
#ifndef TOGGLE_BOOL
#define TOGGLE_BOOL(a) ((a) = 1 - (a))
#endif

/********************  bit comparisons and operations ***************/
#ifndef TEST_BIT
#define TEST_BIT(x, b) (((x) & (1 << (b))) != 0)
#define SET_BIT(x, b) ((x) |= (1 << (b)))
#define CLEAR_BIT(x, b) ((x) &= ~(1 << (b)))
#define TOGGLE_BIT(x, b) ((TEST_BIT(x, b)) ? (CLEAR_BIT(x, b)) : (SET_BIT(x, b)))
#endif

#ifndef M_PI
#define M_PI 3.141592654
#endif

typedef char String[81];

/*********** flush the stdout and stderr output streams *************/
#ifndef flushout
#define flushout fflush(stdout)
#define flusherr fflush(stderr)
#endif

/********** Debugging functions *************************************/
#ifndef error_return
#define error_return(c)     \
    ;                       \
    {                       \
        fprintf(stderr, c); \
        return;             \
    }
#endif

/************************* floating-point random ********************/
#ifndef randf
#define randf() ((float)rand() / (float)RAND_MAX)
#endif

#ifndef SIGN
#define SIGN(x) ((x) >= 0 ? 1 : -1)
#endif

/****************** conversion between degrees and radians **********/
#ifndef DEG2RAD
#define DEG2RAD(x) ((x) / 180.0 * M_PI)
#define RAD2DEG(x) ((x) / M_PI * 180.0)
#endif

/***************** clamp a value to some fixed interval *************/
#ifndef CLAMP
#define CLAMP(x, lo, hi)         \
    {                            \
        if ((x) < (lo)) {        \
            (x) = (lo);          \
        } else if ((x) > (hi)) { \
            (x) = (hi);          \
        }                        \
    }
#endif

#ifndef Byte
#define Byte unsigned char
#endif

#ifndef _RGBC_

class RGBc {
public:
    Byte r, g, b;

    void set(Byte r, Byte g, Byte b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    };

    RGBc(void) {};
    RGBc(Byte r, Byte g, Byte b) { set(r, g, b); };
};

#define _RGBC_
#endif

/************ check if a value lies within a closed interval *********/
#ifndef IN_BOUNDS
#define IN_BOUNDS(x, lo, hi) ((x) >= (lo)AND(x) <= (hi))
#endif

/************ check if a 2D point lies within a 2D box ***************/
#ifndef PT_IN_BOX
#define PT_IN_BOX(x, y, lo_x, hi_x, lo_y, hi_y) \ (IN_BOUNDS(x, lo_x, hi_x) AND IN_BOUNDS(y, lo_y, hi_y))
#endif

/****** check if value lies on proper side of another value     *****/
/*** if side is positive => proper side is positive, else negative **/
#ifndef CHECK_PROPER_SIDE
#define CHECK_PROPER_SIDE(x, val, side) ((side) > 0 ? (x) > (val) : (x) < (val))
#endif

/***** Small value when we want to do a comparison to 'close to zero' *****/
#ifndef FUDGE
#define FUDGE .00001
#endif

/******************* swap two values, using a temp variable *********/
#ifndef SWAP2
#define SWAP2(a, b, t) \
    {                  \
        t = a;         \
        a = b;         \
        b = t;         \
    }
#endif

#define VEC3_TO_ARRAY(v, a) a[0] = v[0], a[1] = v[1], a[2] = v[2]

#endif /* _STDINC_H_ */
