#include <stdbool.h>

// h: handle, i: index, b: buffer*, s: size of buffer
#define get_str_desc(h, i, b, s) libusb_get_string_descriptor_ascii(h, i, b, s)

unsigned char* fill_data(char c, int len);
bool prefix(const char *pre, const char *str);