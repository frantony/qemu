/*
 * mips32r1_soc support
 *
 * based on QEMU Malta board support
 *
 * Copyright (c) 2006 Aurelien Jarno
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qemu-common.h"
#include "cpu.h"
#include "hw/hw.h"
#include "hw/char/serial.h"
#include "net/net.h"
#include "hw/boards.h"
#include "sysemu/block-backend.h"
#include "hw/block/flash.h"
#include "hw/mips/mips.h"
#include "hw/mips/cpudevs.h"
#include "sysemu/char.h"
#include "sysemu/sysemu.h"
#include "sysemu/arch_init.h"
#include "qemu/log.h"
#include "hw/mips/bios.h"
#include "exec/address-spaces.h"
#include "hw/sysbus.h"             /* SysBusDevice */
#include "sysemu/qtest.h"
#include "qemu/error-report.h"
#include "exec/semihost.h"
#include "hw/mips/cps.h"
#include "hw/loader.h"

/* Hardware addresses */
#define UART_ADDRESS  0x1f000900ULL
#define RESET_ADDRESS 0x1fc00000ULL

#define FLASH_SIZE    0x400000

#define TYPE_MIPS_MIPS32R1_SOC "mips32r1-soc"
#define MIPS_MIPS32R1_SOC(obj) OBJECT_CHECK(MIPS32r1State, (obj), TYPE_MIPS_MIPS32R1_SOC)

typedef struct {
    SysBusDevice parent_obj;

    SerialState *uart;
} MIPS32r1State;

static void main_cpu_reset(void *opaque)
{
    MIPSCPU *cpu = opaque;

    cpu_reset(CPU(cpu));
}

static void create_cpu(MIPS32r1State *s, const char *cpu_model)
{
    MIPSCPU *cpu;

    if (cpu_model == NULL) {
        cpu_model = "4Kc";
    }

    cpu = cpu_mips_init(cpu_model);
    if (cpu == NULL) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }

    /* Init internal devices */
    cpu_mips_irq_init_cpu(cpu);
    cpu_mips_clock_init(cpu);
    qemu_register_reset(main_cpu_reset, cpu);
}

static
void mips32r1_soc_init(MachineState *machine)
{
    ram_addr_t ram_size = machine->ram_size;
    char *filename;
    pflash_t *fl;
    MemoryRegion *system_memory = get_system_memory();
    MemoryRegion *ram_high = g_new(MemoryRegion, 1);
    MemoryRegion *ram_low_preio = g_new(MemoryRegion, 1);
    MemoryRegion *bios;
    target_long bios_size = FLASH_SIZE;
    int fl_idx = 0;
    int fl_sectors = bios_size >> 16;
    int be;

    DeviceState *dev = qdev_create(NULL, TYPE_MIPS_MIPS32R1_SOC);
    MIPS32r1State *s = MIPS_MIPS32R1_SOC(dev);

    qdev_init_nofail(dev);

    /* Make sure the first serial port is associated with a device. */
    if (!serial_hds[0]) {
        serial_hds[0] = qemu_chr_new("serial0", "null");
    }

    /* create CPU */
    create_cpu(s, machine->cpu_model);

    /* allocate RAM */
    if (ram_size > (8u << 20)) {
        fprintf(stderr,
                "qemu: Too much memory for this machine: %d MB, maximum 8 MB\n",
                ((unsigned int)ram_size / (1 << 20)));
        exit(1);
    }

    /* register RAM at high address where it is undisturbed by IO */
    memory_region_allocate_system_memory(ram_high, NULL, "mips32r1_soc.ram",
                                         ram_size);
    memory_region_add_subregion(system_memory, 0x80000000, ram_high);

    /* alias for pre IO hole access */
    memory_region_init_alias(ram_low_preio, NULL, "mips32r1_soc_low_preio.ram",
                             ram_high, 0, MIN(ram_size, (256 << 20)));
    memory_region_add_subregion(system_memory, 0, ram_low_preio);

#ifdef TARGET_WORDS_BIGENDIAN
    be = 1;
#else
    be = 0;
#endif

    s->uart = serial_mm_init(system_memory, UART_ADDRESS, 0, NULL,
                             10000000, serial_hds[0], DEVICE_NATIVE_ENDIAN);

    /* Load firmware in flash / BIOS. */
    fl = pflash_cfi01_register(RESET_ADDRESS, NULL, "mips32r1_soc.bios",
                               BIOS_SIZE,
                               NULL,
                               65536, fl_sectors,
                               4, 0x0000, 0x0000, 0x0000, 0x0000, be);
    bios = pflash_cfi01_get_memory(fl);
    fl_idx++;

    /* Load a BIOS image. */
    if (bios_name == NULL) {
        bios_name = BIOS_FILENAME;
    }

    filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, bios_name);
    if (filename) {
        bios_size = load_image_targphys(filename, RESET_ADDRESS,
                                        BIOS_SIZE);
        g_free(filename);
    } else {
        bios_size = -1;
    }

    if ((bios_size < 0 || bios_size > BIOS_SIZE) &&
        !qtest_enabled()) {
        error_report("Could not load MIPS bios '%s'", bios_name);
        exit(1);
    }

    /* In little endian mode the 32bit words in the bios are swapped,
       a neat trick which allows bi-endian firmware. */
#ifndef TARGET_WORDS_BIGENDIAN
    {
        uint32_t *end, *addr = rom_ptr(RESET_ADDRESS);
        if (!addr) {
            addr = memory_region_get_ram_ptr(bios);
        }
        end = (void *)addr + MIN(bios_size, 0x3e0000);
        while (addr < end) {
            bswap32s(addr);
            addr++;
        }
    }
#endif
}

static int mips32r1_soc_sysbus_device_init(SysBusDevice *sysbusdev)
{
    return 0;
}

static void mips32r1_soc_class_init(ObjectClass *klass, void *data)
{
    SysBusDeviceClass *k = SYS_BUS_DEVICE_CLASS(klass);

    k->init = mips32r1_soc_sysbus_device_init;
}

static const TypeInfo mips32r1_soc_device = {
    .name          = TYPE_MIPS_MIPS32R1_SOC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(MIPS32r1State),
    .class_init    = mips32r1_soc_class_init,
};

static void mips32r1_soc_machine_init(MachineClass *mc)
{
    mc->desc = "MIPS32r1 SoC";
    mc->init = mips32r1_soc_init;
    mc->max_cpus = 1;
    mc->is_default = 0;
}

DEFINE_MACHINE("mips32r1-soc", mips32r1_soc_machine_init)

static void mips32r1_soc_register_types(void)
{
    type_register_static(&mips32r1_soc_device);
}

type_init(mips32r1_soc_register_types)
