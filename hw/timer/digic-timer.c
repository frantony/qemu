/*
 * QEMU model of the Canon Digic timer block.
 *
 * Copyright (C) 2013 Antony Pavlov <antonynpavlov@gmail.com>
 *
 * This model is based on reverse engineering efforts
 * made by CHDK (http://chdk.wikia.com) and
 * Magic Lantern (http://www.magiclantern.fm) projects
 * contributors.
 *
 * See "Timer/Clock Module" docs here:
 *   http://magiclantern.wikia.com/wiki/Register_Map
 *
 * The QEMU model of the OSTimer in PKUnity SoC by Guan Xuetao
 * is used as a template.
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

#include "hw/sysbus.h"
#include "hw/ptimer.h"
#include "qemu/main-loop.h"

#include "hw/timer/digic-timer.h"

#ifdef DEBUG_DIGIC_TIMER
#define DPRINTF(fmt, ...) printf("%s: " fmt , __func__, ## __VA_ARGS__)
#else
#define DPRINTF(fmt, ...) do {} while (0)
#endif

# define DIGIC_TIMER_CONTROL 0x00
# define DIGIC_TIMER_VALUE 0x0c

static uint64_t digic_timer_read(void *opaque, hwaddr offset,
        unsigned size)
{
    DigicTimerState *s = opaque;
    uint32_t ret = 0;

    switch (offset) {
    case DIGIC_TIMER_VALUE:
        ret = (uint32_t)ptimer_get_count(s->ptimer);
        ret = ret & 0xffff;
        break;
    default:
        DPRINTF("Bad offset %x\n", (int)offset);
    }

    DPRINTF("offset 0x%x, value 0x%x\n", offset, ret);
    return ret;
}

static void digic_timer_write(void *opaque, hwaddr offset,
        uint64_t value, unsigned size)
{
    DigicTimerState *s = opaque;

    /* FIXME: just now we ignore timer enable bit */
    ptimer_set_limit(s->ptimer, 0x0000ffff, 1);
    ptimer_run(s->ptimer, 1);
}

static const MemoryRegionOps digic_timer_ops = {
    .read = digic_timer_read,
    .write = digic_timer_write,
    .impl = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void digic_timer_tick(void *opaque)
{
    DigicTimerState *s = opaque;

    ptimer_run(s->ptimer, 1);
}

static void digic_timer_init(Object *obj)
{
    DigicTimerState *s = DIGIC_TIMER(obj);

    s->bh = qemu_bh_new(digic_timer_tick, s);
    s->ptimer = ptimer_init(s->bh);

    /* FIXME: there is no documentation on Digic timer
     * frequency setup so let's it always run on 1 MHz
     * */
    ptimer_set_freq(s->ptimer, 1 * 1000 * 1000);

    memory_region_init_io(&s->iomem, OBJECT(s), &digic_timer_ops, s,
            TYPE_DIGIC_TIMER, 0x100);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);
}

static const TypeInfo digic_timer_info = {
    .name = TYPE_DIGIC_TIMER,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(DigicTimerState),
    .instance_init = digic_timer_init,
};

static void digic_timer_register_type(void)
{
    type_register_static(&digic_timer_info);
}

type_init(digic_timer_register_type)
