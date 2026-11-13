#ifndef ER_OSAL_ZEPHYR_H
#define ER_OSAL_ZEPHYR_H

#include "er/osal.h"

/* Zephyr supplies the monotonic clock and sleep primitive.  The application
 * still owns its threads; this adapter only implements the shared timing
 * contract used by the control logic. */
er_osal_clock_t er_osal_zephyr_clock(void);

#endif
