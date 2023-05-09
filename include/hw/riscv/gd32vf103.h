/*
 * QEMU GigaDevice GD32VF103 MCU
 *
 * Copyright (c) 2023 Austin Lund <austin.lund@gmail.com>
 *
 * This work is licensed under the terms of the GNU GPLv2 or later.
 * See the COPYING file in the top-level directory.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef HW_GD32VF103_H
#define HW_GD32VF103_H

#include "hw/sysbus.h"
#include "hw/riscv/riscv_hart.h"

#define FLASH_BASE_ADDRESS 0x08000000
#define SRAM_BASE_ADDRESS 0x20000000

#define TYPE_GD32VF103 "gd32vf103-mcu"
OBJECT_DECLARE_SIMPLE_TYPE(GD32VF103State, GD32VF103)

struct GD32VF103State {
    /* private */
    SysBusDevice parent;

    /* public */
    RISCVCPU cpu;

    MemoryRegion sram;
    MemoryRegion flash;
    MemoryRegion flash_alias;
};

#endif
