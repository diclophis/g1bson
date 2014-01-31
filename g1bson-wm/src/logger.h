/*--
  --*/

#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <stdio.h>

#ifndef MPWM_NODEBUG

#define TRACE(...) printf("TRACE: " __VA_ARGS__)
#define DBG(...) printf("DBG:   " __VA_ARGS__)
#define ERR(...) fprintf(stderr, "ERROR: " __VA_ARGS__)

#else

#define TRACE(...) /* __VA_ARGS__ */
#define DBG(...) /* __VA_ARGS__ */
#define ERR(...) /* __VA_ARGS__ */

#endif

#endif
