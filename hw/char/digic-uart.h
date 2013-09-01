#ifndef HW_CHAR_DIGIC_UART_H
#define HW_CHAR_DIGIC_UART_H

#include "hw/sysbus.h"
#include "qemu/typedefs.h"

#define TYPE_DIGIC_UART "digic-uart"
#define DIGIC_UART(obj) \
    OBJECT_CHECK(DigicUartState, (obj), TYPE_DIGIC_UART)

enum {
    R_TX = 0x00,
    R_RX,
    R_ST = (0x14 >> 2),
    R_MAX
};

typedef struct DigicUartState {
    SysBusDevice parent_obj;

    MemoryRegion regs_region;
    CharDriverState *chr;

    uint32_t regs[R_MAX];
} DigicUartState;

#endif /* HW_CHAR_DIGIC_UART_H */
