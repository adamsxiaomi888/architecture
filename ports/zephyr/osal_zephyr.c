#include "osal_zephyr.h"

#include <limits.h>
#include <zephyr/kernel.h>

static int now_us(void *ctx, uint64_t *out)
{
    (void)ctx;
    if (!out) {
        return ER_OSAL_EINVAL;
    }

    /* k_uptime_ticks() is monotonic and k_ticks_to_us_near64() avoids the
     * 32-bit overflow that would occur when converting long-running systems
     * through k_uptime_get(). */
    *out = k_ticks_to_us_near64((uint64_t)k_uptime_ticks());
    return ER_OSAL_OK;
}

static int wait_until(void *ctx, uint64_t deadline_us)
{
    (void)ctx;
    uint64_t now;
    int rc = now_us(NULL, &now);
    if (rc != ER_OSAL_OK) {
        return rc;
    }
    if (now >= deadline_us) {
        return ER_OSAL_OK;
    }

    /* K_USEC accepts a 64-bit duration.  Keep each sleep bounded so this
     * remains safe on Zephyr versions whose underlying timeout is narrower. */
    uint64_t remaining = deadline_us - now;
    while (remaining) {
        uint64_t slice = remaining;
        if (slice > (uint64_t)INT32_MAX) {
            slice = (uint64_t)INT32_MAX;
        }
        k_sleep(K_USEC(slice));
        rc = now_us(NULL, &now);
        if (rc != ER_OSAL_OK || now >= deadline_us) {
            return rc;
        }
        remaining = deadline_us - now;
    }
    return ER_OSAL_OK;
}

er_osal_clock_t er_osal_zephyr_clock(void)
{
    er_osal_clock_t clock = { now_us, wait_until, NULL };
    return clock;
}
