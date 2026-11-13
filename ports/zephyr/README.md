# Zephyr OSAL port

`osal_zephyr.c` adapts Zephyr's monotonic kernel clock and sleep primitive to
the common `er_osal_clock_t` contract. Application threads remain owned by the
product integration; shared control code only receives an `er_osal_clock_t`.

Use it from a Zephyr application by adding `ports/zephyr` to the include path,
compiling `ports/zephyr/osal_zephyr.c`, and linking `src/osal.c`. A typical
control thread starts with:

```c
er_periodic_t loop;
er_osal_clock_t clock = er_osal_zephyr_clock();
er_periodic_init(&loop, &clock, 10000); /* 10 ms */
for (;;) {
    er_periodic_report_t report;
    if (er_periodic_wait(&loop, &report) != ER_OSAL_OK) {
        /* enter the product's safe state */
        break;
    }
    control_step(&report);
}
```

The port needs Zephyr's `kernel.h` and is therefore built by a Zephyr west
application, not by the host CMake test suite. The same control code can be
run by the existing POSIX simulator for deterministic tests.
