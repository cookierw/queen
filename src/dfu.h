#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

void init_ctx();
void aquire_device();
void release_device();
void reset_device();

struct libusb_transfer* create_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, unsigned char* buffer, int timeout
);

int async_ctrl_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, unsigned char *data, unsigned int timeout
);

void transfer_cb(struct libusb_transfer* transfer);
void transfer_wait_for_completion(struct libusb_transfer* transfer);

int my_control_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, uint16_t wLength, unsigned char* data, 
    unsigned int timeout, unsigned char* user_data
);

int no_error_ctrl_transfer(
    uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, 
    uint16_t wIndex, unsigned char* data, int length, unsigned int timeout
);

int stall();
int leak();
int no_leak();
int usb_req_stall();
int usb_req_leak();
int usb_req_no_leak();

char* get_serial_string();