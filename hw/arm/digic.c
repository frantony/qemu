/*
 * QEMU model of the Canon SoC.
 *
 * Copyright (C) 2013 Antony Pavlov <antonynpavlov@gmail.com>
 *
 * This model is based on reverse engineering efforts
 * made by CHDK (http://chdk.wikia.com) and
 * Magic Lantern (http://www.magiclantern.fm) projects
 * contributors.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "exec/address-spaces.h"
#include "hw/sysbus.h"

#define DIGIC4_TIMER0    0xc0210000
#define DIGIC4_TIMER1    0xc0210100
#define DIGIC4_TIMER2    0xc0210200
#define DIGIC4_UART      0xc0800000

typedef struct DigicState {
    ARMCPU *cpu;
} DigicState;

DigicState *digic4_create(void)
{
    DigicState *s = g_new(DigicState, 1);

    s->cpu = cpu_arm_init("arm946");
    if (!s->cpu) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }

    sysbus_create_simple("digic-timer", DIGIC4_TIMER0, NULL);
    sysbus_create_simple("digic-timer", DIGIC4_TIMER1, NULL);
    sysbus_create_simple("digic-timer", DIGIC4_TIMER2, NULL);

    sysbus_create_simple("digic-uart", DIGIC4_UART, NULL);

    return s;
}
