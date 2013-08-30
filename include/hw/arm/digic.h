/*
 * Misc DIGIC declarations
 *
 * Copyright (C) 2013 Antony Pavlov <antonynpavlov@gmail.com>
 *
 */

#ifndef __DIGIC_H__
#define __DIGIC_H__

#include "cpu-qom.h"

#define DIGIC4_TIMER0    0xc0210000
#define DIGIC4_TIMER1    0xc0210100
#define DIGIC4_TIMER2    0xc0210200

typedef struct DigicState {
    ARMCPU *cpu;
} DigicState;

DigicState *digic4_init(void);

#endif /* __DIGIC_H__ */
