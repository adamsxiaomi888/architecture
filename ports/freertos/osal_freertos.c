#include "osal_freertos.h"
#include <stdint.h>

static int now_us(void *ctx, uint64_t *out)
{
    (void)ctx;
    if (!out) return ER_OSAL_EINVAL;
    /* Extend the tick counter in the application port if wrap matters. */
    TickType_t ticks = xTaskGetTickCount();
    uint64_t hz = (uint64_t)configTICK_RATE_HZ;
    if (!hz || (uint64_t)ticks > UINT64_MAX / 1000000u * hz)
        return ER_OSAL_EOVERFLOW;
    *out = ((uint64_t)ticks * 1000000u) / hz;
    return ER_OSAL_OK;
}

static int wait_until(void *ctx, uint64_t deadline_us)
{
    (void)ctx;
    uint64_t now;
    int rc = now_us(0, &now);
    if (rc != ER_OSAL_OK) return rc;
    if (now >= deadline_us) return ER_OSAL_OK;
    uint64_t remaining = deadline_us - now;
    uint64_t hz = (uint64_t)configTICK_RATE_HZ;
    uint64_t ticks = (remaining * hz + 999999u) / 1000000u;
    if (!ticks) ticks = 1;
    if (ticks > (uint64_t)portMAX_DELAY) ticks = (uint64_t)portMAX_DELAY;
    vTaskDelay((TickType_t)ticks);
    return ER_OSAL_OK;
}

er_osal_clock_t er_osal_freertos_clock(void)
{
    er_osal_clock_t clock = { now_us, wait_until, 0 };
    return clock;
}
