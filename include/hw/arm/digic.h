/*
 * Misc DIGIC declarations
 *
 * Copyright (C) 2013 Antony Pavlov <antonynpavlov@gmail.com>
 *
 */

#ifndef __DIGIC_H__
#define __DIGIC_H__

#include "cpu-qom.h"

typedef struct DigicState {
    ARMCPU *cpu;
} DigicState;

DigicState *digic4_init(void);

#endif /* __DIGIC_H__ */
