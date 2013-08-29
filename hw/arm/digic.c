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
#include "hw/boards.h"
#include "hw/block/flash.h"
#include "hw/loader.h"

#define DIGIC4_TIMER0    0xc0210000
#define DIGIC4_TIMER1    0xc0210100
#define DIGIC4_TIMER2    0xc0210200
#define DIGIC4_UART      0xc0800000
#define DIGIC4_ROM0      0xf0000000
#define DIGIC4_ROM1      0xf8000000
# define DIGIC4_ROM_MAX_SIZE      0x08000000

typedef struct DigicState {
    ARMCPU *cpu;
    MemoryRegion ram;
} DigicState;

typedef struct {
    hwaddr ram_size;
    void (*add_rom0)(DigicState *, hwaddr, const char *);
    const char *rom0_filename;
    void (*add_rom1)(DigicState *, hwaddr, const char *);
    const char *rom1_filename;
    hwaddr start_addr;
} DigicBoard;

static DigicState *digic4_create(void)
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

static void digic4_setup_ram(DigicState *s, hwaddr ram_size)
{
    memory_region_init_ram(&s->ram, NULL, "ram", ram_size);
    memory_region_add_subregion(get_system_memory(), 0, &s->ram);
    vmstate_register_ram_global(&s->ram);
}

static void init_digic4_board(DigicBoard *board)
{
    DigicState *s = digic4_create();

    digic4_setup_ram(s, board->ram_size);

    if (board->add_rom0) {
        board->add_rom0(s, DIGIC4_ROM0, board->rom0_filename);
    }

    if (board->add_rom1) {
        board->add_rom1(s, DIGIC4_ROM1, board->rom1_filename);
    }

    s->cpu->env.regs[15] = board->start_addr;
}

static void digic4_add_k8p3215uqb_rom(DigicState *s, hwaddr addr,
        const char *filename)
{
#define FLASH_K8P3215UQB_SIZE (4 * 1024 * 1024)
#define FLASH_K8P3215UQB_SECTOR_SIZE (64 * 1024)

    target_long rom_size;

    /*
     * Samsung K8P3215UQB:
     *  * AMD command set;
     *  * multiple sector size: some sectors are 8K the other ones are 64K.
     * Alas! The pflash_cfi02_register() function creates a flash
     * device with unified sector size.
     */
    pflash_cfi02_register(addr, NULL, "pflash", FLASH_K8P3215UQB_SIZE,
            NULL, FLASH_K8P3215UQB_SECTOR_SIZE,
            FLASH_K8P3215UQB_SIZE / FLASH_K8P3215UQB_SECTOR_SIZE,
            DIGIC4_ROM_MAX_SIZE / FLASH_K8P3215UQB_SIZE,
            4,
            0x00EC, 0x007E, 0x0003, 0x0001,
            0x0555, 0x2aa, 0);

    /* FIXME: this load a __local__ file */
    if (filename) {
        rom_size = load_image_targphys(filename, addr, FLASH_K8P3215UQB_SIZE);
        if (rom_size < 0 || rom_size > FLASH_K8P3215UQB_SIZE) {
            fprintf(stderr, "Could not load rom image '%s'\n", filename);
            exit(1);
        }
    }
}

static DigicBoard a1100_board = {
    .ram_size = 64 * 1024 * 1024,
    .add_rom1 = digic4_add_k8p3215uqb_rom,
    .rom1_filename = "canon-a1100-rom1.bin",
    .start_addr = DIGIC4_ROM1,
};

static void init_a1100(QEMUMachineInitArgs *args)
{
    init_digic4_board(&a1100_board);
}

static QEMUMachine canon_a1100 = {
    .name = "canon-a1100",
    .desc = "Canon PowerShot A1100 IS",
    .init = &init_a1100,
};

static void digic_register_machines(void)
{
    qemu_register_machine(&canon_a1100);
}

machine_init(digic_register_machines)
