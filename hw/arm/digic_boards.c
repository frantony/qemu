#include "hw/boards.h"
#include "exec/address-spaces.h"
#include "hw/arm/digic.h"

typedef struct DigicBoardState {
    DigicState *digic;
    MemoryRegion ram;
} DigicBoardState;

    MemoryRegion ram;

typedef struct DigicBoard {
    hwaddr ram_size;
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

    s->digic->cpu->env.regs[15] = board->start_addr;
}

static DigicBoard a1100_board = {
    .ram_size = 64 * 1024 * 1024,
    /* CHDK recommends this address for ROM disassembly */
    .start_addr = 0xffc00000,
};

static void canon_a1100_init(QEMUMachineInitArgs *args)
{
    digic4_board_init(&a1100_board);
}

static QEMUMachine canon_a1100 = {
    .name = "canon-a1100",
    .desc = "Canon PowerShot A1100 IS",
    .init = &canon_a1100_init,
};

static void digic_register_machines(void)
{
    qemu_register_machine(&canon_a1100);
}

machine_init(digic_register_machines)
