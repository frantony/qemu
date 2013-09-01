/*
 * Canon DIGIC timer block declarations.
 *
 * Copyright (C) 2013 Antony Pavlov <antonynpavlov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HW_TIMER_DIGIC_TIMER_H
#define HW_TIMER_DIGIC_TIMER_H

#include "hw/sysbus.h"
#include "qemu/typedefs.h"
#include "hw/ptimer.h"

#define TYPE_DIGIC_TIMER "digic-timer"
#define DIGIC_TIMER(obj) OBJECT_CHECK(DigicTimerState, (obj), TYPE_DIGIC_TIMER)

typedef struct DigicTimerState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    QEMUBH *bh;
    ptimer_state *ptimer;
} DigicTimerState;

#endif /* HW_TIMER_DIGIC_TIMER_H */
