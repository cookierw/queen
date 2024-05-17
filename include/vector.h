#include <stdint.h>

typedef struct vector64 {
    uint64_t* data;
    int size;
    int capacity;
} v64;

typedef struct vector32 {
    uint32_t* data;
    int size;
    int capacity;
} v32;

typedef struct vector16 {
    uint16_t* data;
    int size;
    int capacity;
} v16;

typedef struct vector8 {
    uint8_t* data;
    int size;
    int capacity;
} v8;

v64 v64_new(int capacity);
v32 v32_new(int capacity);
v16 v16_new(int capacity);
v8   v8_new(int capacity);

void v64_init(v64* v, int capacity);
void v32_init(v32* v, int capacity);
void v16_init(v16* v, int capacity);
void  v8_init(v8*  v, int capacity);

void v64_push(v64* v, uint64_t element);
void v32_push(v32* v, uint32_t element);
void v16_push(v16* v, uint16_t element);
void  v8_push(v8*  v, uint8_t  element);

void v64_push_back(v64* v, uint64_t element);
void v32_push_back(v32* v, uint32_t element);
void v16_push_back(v16* v, uint16_t element);
void  v8_push_back(v8*  v, uint8_t  element);

uint64_t v64_pop(v64* v);
uint32_t v32_pop(v32* v);
uint16_t v16_pop(v16* v);
uint8_t   v8_pop(v8*  v);

uint64_t v64_value(v64* v, int index);
uint32_t v32_value(v32* v, int index);
uint16_t v16_value(v16* v, int index);
uint8_t   v8_value(v8*  v, int index);

void v64_resize(v64* v, int new_capacity);
void v32_resize(v32* v, int new_capacity);
void v16_resize(v16* v, int new_capacity);
void  v8_resize(v8*  v, int new_capacity);

void v64_append(v64* dest, v64 source);
void  v8_append(v8*  dest,  v8 source);
void v8_push_u32(v8* v, uint32_t element);
v8 v64_convert_v8(v64 source);
v8 v8_range_of(v8* v, int start);
v8 v8_zeros(int amount);