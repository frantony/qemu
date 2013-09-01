/*
 * Misc DIGIC declarations
 *
 * Copyright (C) 2013 Antony Pavlov <antonynpavlov@gmail.com>
 *
 */

#ifndef __DIGIC_H__
#define __DIGIC_H__

#include "cpu-qom.h"

#include "hw/timer/digic-timer.h"
#include "hw/char/digic-uart.h"

#define DIGIC4_NB_TIMERS 3

#define DIGIC4_TIMER(n)    (0xc0210000 + n * 0x100)
#define DIGIC4_UART      0xc0800000

#define TYPE_DIGIC "digic"

#define DIGIC(obj) OBJECT_CHECK(DigicState, (obj), TYPE_DIGIC)

typedef struct DigicState {
    Object parent_obj;

    ARMCPU cpu;

    DigicTimerState timer[DIGIC4_NB_TIMERS];
    DigicUartState uart;
} DigicState;

//DigicState *digic4_init(void);

#endif /* __DIGIC_H__ */
