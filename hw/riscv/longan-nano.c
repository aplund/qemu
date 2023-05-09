/*
 * QEMU Longan Nano board
 *
 * Copyright (c) 2023 Austin Lund
 *
 * This work is licensed under the terms of the GNU GPLv2 or later.
 * See the COPYING file in the top-level directory.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object.h"
#include "hw/riscv/gd32vf103.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "qemu/datadir.h"
#include "qemu/error-report.h"

struct LonganNanoMachineState {
    /*< private >*/
    MachineState parent_obj;
    /*< public >*/
    GD32VF103State mcu;
};
typedef struct LonganNanoMachineState LonganNanoMachineState;

struct LonganNanoMachineClass {
    /*< private >*/
    MachineClass parent_class;
    /*< public >*/
    const char *mcu_type;
};
typedef struct LonganNanoMachineClass LonganNanoMachineClass;

#define TYPE_LONGAN_NANO_MACHINE \
        MACHINE_TYPE_NAME("longan-nano")
DECLARE_OBJ_CHECKERS(LonganNanoMachineState, LonganNanoMachineClass,
                     LONGAN_NANO_MACHINE, TYPE_LONGAN_NANO_MACHINE)

static void longan_nano_machine_init(MachineState *machine)
{
    LonganNanoMachineClass *lnmc = LONGAN_NANO_MACHINE_GET_CLASS(machine);
    LonganNanoMachineState *lnms = LONGAN_NANO_MACHINE(machine);

    g_autofree char *filename = NULL;
    int bytes_loaded;

    object_initialize_child(OBJECT(machine), "mcu", &lnms->mcu, lnmc->mcu_type);
    sysbus_realize(SYS_BUS_DEVICE(&lnms->mcu), &error_abort);

    if (machine->firmware) {
        filename = qemu_find_file(QEMU_FILE_TYPE_BIOS, machine->firmware);
        if (filename == NULL) {
            error_report("Unable to find %s", machine->firmware);
            exit(1);
        }
        bytes_loaded = load_image_mr(filename, &(lnms->mcu).flash);
        if (bytes_loaded < 0) {
            error_report("Unable to load firmware image %s", machine->firmware);
            exit(1);
        }
    }
}

static void longan_nano_class_init(ObjectClass *oc, void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    LonganNanoMachineClass *lnmc = LONGAN_NANO_MACHINE_CLASS(oc);

    mc->init = longan_nano_machine_init;
    mc->desc = "RISC-V Longan Nano board";

    lnmc->mcu_type = TYPE_GD32VF103;
    mc->default_cpus = 1;
    mc->min_cpus = 1;
    mc->max_cpus = 1;
    mc->no_floppy = 1;
    mc->no_cdrom = 1;
    mc->no_parallel = 1;
}

static const TypeInfo longan_nano_machine_types[] = {
    {
        .name = TYPE_LONGAN_NANO_MACHINE,
        .parent = TYPE_MACHINE,
        .instance_size = sizeof(LonganNanoMachineState),
        .class_size = sizeof(LonganNanoMachineClass),
        .class_init = longan_nano_class_init,
    }
};

DEFINE_TYPES(longan_nano_machine_types)
