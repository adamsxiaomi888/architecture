#ifndef ER_OSAL_POSIX_H
#define ER_OSAL_POSIX_H
#include "er/osal.h"
/* No global state or thread creation. Caller owns the periodic task. */
er_osal_clock_t er_osal_posix_clock(void);
#endif
