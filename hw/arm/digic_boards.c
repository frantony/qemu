#include "hw/boards.h"
#include "exec/address-spaces.h"
#include "hw/block/flash.h"
#include "hw/loader.h"
#include "hw/arm/digic.h"

#define DIGIC4_ROM0      0xf0000000
#define DIGIC4_ROM1      0xf8000000
# define DIGIC4_ROM_MAX_SIZE      0x08000000

typedef struct DigicBoardState {
    DigicState *digic;
    MemoryRegion ram;
} DigicBoardState;

    MemoryRegion ram;

typedef struct DigicBoard {
    hwaddr ram_size;
    void (*add_rom0)(DigicBoardState *, hwaddr, const char *);
    const char *rom0_filename;
    void (*add_rom1)(DigicBoardState *, hwaddr, const char *);
    const char *rom1_filename;
    hwaddr start_addr;
} DigicBoard;

static void digic4_board_setup_ram(DigicBoardState *s, hwaddr ram_size)
{
    memory_region_init_ram(&s->ram, NULL, "ram", ram_size);
    memory_region_add_subregion(get_system_memory(), 0, &s->ram);
    vmstate_register_ram_global(&s->ram);
}

static void digic4_board_init(DigicBoard *board)
{
    DigicBoardState *s = g_new(DigicBoardState, 1);

    s->digic = digic4_init();

    digic4_board_setup_ram(s, board->ram_size);

    if (board->add_rom0) {
        board->add_rom0(s, DIGIC4_ROM0, board->rom0_filename);
    }

    if (board->add_rom1) {
        board->add_rom1(s, DIGIC4_ROM1, board->rom1_filename);
    }

    s->digic->cpu->env.regs[15] = board->start_addr;
}

static void digic4_add_k8p3215uqb_rom(DigicBoardState *s, hwaddr addr,
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
    digic4_board_init(&a1100_board);
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
