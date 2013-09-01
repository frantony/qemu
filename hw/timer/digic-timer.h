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
