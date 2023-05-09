/*
 * QEMU GigaDevice GD32VF103 MCU
 *
 * Copyright (c) 2023 Austin Lund <austin.lund@gmail.com>
 *
 * This work is licensed under the terms of the GNU GPLv2 or later.
 * See the COPYING file in the top-level directory.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qapi/error.h"

#include "exec/memory.h"
#include "exec/address-spaces.h"

#include "hw/sysbus.h"
#include "hw/riscv/gd32vf103.h"

#include "target/riscv/cpu.h"

#define FLASH_SIZE (128 * 1024)
#define SRAM_SIZE (32 * 1024)

static RISCVException bumblebee_csr_unimp_predicate_fn(CPURISCVState *env,
                                                int csrno)
{
    qemu_log_mask(LOG_UNIMP, "%s (csrno = %04x)", __func__, csrno);

    /* TODO: Test for bumblebee */
    return RISCV_EXCP_NONE;
}

static RISCVException bumblebee_csr_unimp_read_fn(CPURISCVState *env, int csrno,
                                            target_ulong *ret_value)
{
    qemu_log_mask(LOG_UNIMP, "%s (csrno = %04x): %08x", __func__, 
                  csrno, *ret_value);
    /* TODO: Test for bumblebee */
    return RISCV_EXCP_NONE;
}

static RISCVException bumblebee_csr_unimp_write_fn(CPURISCVState *env, int csrno,
                                             target_ulong new_value)
{
    qemu_log_mask(LOG_UNIMP, "%s (csrno = %04x): %08x", __func__, 
                  csrno, new_value);
    /* TODO: Test for bumblebee */
    return RISCV_EXCP_NONE;
}

/* Bumblebee Core Manual 
 *
 * https://github.com/nucleisys/Bumblebee_Core_Doc/blob/c8601e42b7400a733ec546a155dde840ea3a6f19/Bumblebee%20Core%20Architecture%20Manual.pdf
 */

static riscv_csr_operations bumblebee_unimp_csr_operation = {
   .name = "Bumblebee Custom CSR unimplemented",
   .predicate = bumblebee_csr_unimp_predicate_fn,
   .read = bumblebee_csr_unimp_read_fn,
   .write = bumblebee_csr_unimp_write_fn,
};

/* 
0x320 MRW mcountinhibit Customized register for counters on & off
0x7c3 MRO mnvec NMI Entry Address
0x7c4 MRW msubm Customized Register Storing Type of Trap
0x7d0 MRW mmisc_ctl Customized Register holding NMI Handler Entry Address
0x7d6 MRW msavestatus Customized Register holding the value of mstatus
0x7d7 MRW msaveepc1 Customized Register holding the value of mepc for the first-level preempted NMI or Exception.
0x7d8 MRW msavecause1 Customized Register holding the value of mcause for the first-level preempted NMI or Exception.
0x7d9 MRW msaveepc2 Customized Register holding the value of mepc for the second-level preempted NMI or Exception.
0x7da MRW msavecause2 Customized Register holding the value of mcause for the second-level preempted NMI or Exception.
0x7eb MRW pushmsubm Push msubm to stack
0x7ec MRW mtvt2 ECLIC non-vectored interrupt handler address register
0x7ed MRW jalmnxti Jumping to next interrupt handler address and interrupt-enable register
0x7ee MRW pushmcause Push mcause to stack
0x7ef MRW pushmepc Push mepc to stack
0x811 MRW sleepvalue WFI Sleep Mode Register
0x812 MRW txevt Send Event Register
0x810 MRW wfe Wait for Event Control Register

*/

static void gd32vf103_realize(DeviceState *dev, Error **errp) {
  GD32VF103State *s = GD32VF103(dev);
  MemoryRegion *system_memory = get_system_memory();

  /* CPU */
  object_initialize_child(OBJECT(dev), "cpu", &s->cpu,
                          TYPE_RISCV_CPU_BUMBLEBEE);
  // These should go into a sperate file
  object_property_set_uint(OBJECT(&s->cpu), "resetvec", 0x0, &error_abort);
  riscv_set_csr_ops(0x320, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7c3, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7c4, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7d0, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7d6, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7d7, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7d8, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7d9, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7da, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7eb, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7ec, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7ed, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7ee, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x7ef, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x811, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x812, &bumblebee_unimp_csr_operation);
  riscv_set_csr_ops(0x810, &bumblebee_unimp_csr_operation);
  qdev_realize(DEVICE(&s->cpu), NULL, &error_abort);

  /* Flash memory */
  memory_region_init_rom(&s->flash, OBJECT(dev), "GD32VF103.flash", FLASH_SIZE,
                         &error_fatal);
  memory_region_init_alias(&s->flash_alias, OBJECT(dev),
                           "GD32VF103.flash.alias", &s->flash, 0, FLASH_SIZE);
  memory_region_add_subregion(system_memory, FLASH_BASE_ADDRESS, &s->flash);
  memory_region_add_subregion(system_memory, 0, &s->flash_alias);

  /* SRAM memory */
  memory_region_init_ram(&s->sram, NULL, "GD32VF103.sram", SRAM_SIZE,
                         &error_fatal);
  memory_region_add_subregion(system_memory, SRAM_BASE_ADDRESS, &s->sram);
}

static void gd32vf103_class_init(ObjectClass *klass, void *class_data) {
  DeviceClass *dc = DEVICE_CLASS(klass);

  dc->realize = gd32vf103_realize;
  /* Reason: Mapped at fixed location on the system bus */
  dc->user_creatable = false;
}

static const TypeInfo gd32vf103_info = {
    .name = TYPE_GD32VF103,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(GD32VF103State),
    .class_init = gd32vf103_class_init,
};

static void gd32vf103_register_types(void) {
  type_register_static(&gd32vf103_info);
}

type_init(gd32vf103_register_types)
