#include <stdlib.h>

// random numbers for windows
#ifdef WIN32
inline double drand48( )
{
    return ((double)rand()) / (RAND_MAX+1.0);
}
#endif
