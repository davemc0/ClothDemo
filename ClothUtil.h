#include <stdlib.h>

// Random numbers for windows
#ifdef WIN32
inline double drand48() { return ((double)rand()) / (RAND_MAX + 1.0); }
#endif
