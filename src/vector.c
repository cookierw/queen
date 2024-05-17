#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

v64 v64_new(int capacity) {
    v64 v;
    v.data = malloc(sizeof(uint64_t) * capacity);
    v.size = 0;
    v.capacity = capacity;
    return v;
}

v32 v32_new(int capacity) {
    v32 v;
    v.data = malloc(sizeof(uint32_t) * capacity);
    v.size = 0;
    v.capacity = capacity;
    return v;
}

v16 v16_new(int capacity) {
    v16 v;
    v.data = malloc(sizeof(uint16_t) * capacity);
    v.size = 0;
    v.capacity = capacity;
    return v;
}

v8 v8_new(int capacity) {
    v8 v;
    v.data = malloc(sizeof(uint8_t) * capacity);
    v.size = 0;
    v.capacity = capacity;
    return v;
}

void v64_init(v64* v, int capacity) {
    v->data = malloc(sizeof(uint64_t) * capacity);
    v->size = 0;
    v->capacity = capacity;
}

void v32_init(v32* v, int capacity) {
    v->data = malloc(sizeof(uint32_t) * capacity);
    v->size = 0;
    v->capacity = capacity;
}

void v16_init(v16* v, int capacity) {
    v->data = malloc(sizeof(uint16_t) * capacity);
    v->size = 0;
    v->capacity = capacity;
}

void v8_init(v8* v, int capacity) {
    v->data = malloc(sizeof(uint8_t) * capacity);
    v->size = 0;
    v->capacity = capacity;
}

void v64_push(v64* v, uint64_t element) {
    if (v->size == v->capacity) {
        v64_resize(v, v->capacity * 2);
    }
    v->data[v->size] = element;
    v->size++;
}

void v32_push(v32* v, uint32_t element) {
    if (v->size == v->capacity) {
        v32_resize(v, v->capacity * 2);
    }
    v->data[v->size] = element;
    v->size++;
}

void v16_push(v16* v, uint16_t element) {
    if (v->size == v->capacity) {
        v16_resize(v, v->capacity * 2);
    }
    v->data[v->size] = element;
    v->size++;
}

void v8_push(v8*  v, uint8_t  element) {
    if (v->size == v->capacity) {
        v8_resize(v, v->capacity * 2);
    }
    v->data[v->size] = element;
    v->size++;
}

void v64_push_back(v64* v, uint64_t element) {
    if (v->size == v->capacity) {
        v64_resize(v, v->capacity * 2);
    }
    uint64_t* data = v->data; 
    v->data = malloc(sizeof(uint64_t) * v->capacity);
    memcpy(v->data, &element, sizeof(element));
    memcpy(v->data + sizeof(element), data, (sizeof(uint64_t) * v->size));
    free(data);
    v->size++;
}

void v32_push_back(v32* v, uint32_t element) {
    if (v->size == v->capacity) {
        v32_resize(v, v->capacity * 2);
    }
    uint32_t* data = v->data; 
    v->data = malloc(sizeof(uint32_t) * v->capacity);
    memcpy(v->data, &element, sizeof(element));
    memcpy(v->data + sizeof(element), data, (sizeof(uint32_t) * v->size));
    free(data);
    v->size++;
}

void v16_push_back(v16* v, uint16_t element) {
    if (v->size == v->capacity) {
        v16_resize(v, v->capacity * 2);
    }
    uint16_t* data = v->data; 
    v->data = malloc(sizeof(uint16_t) * v->capacity);
    memcpy(v->data, &element, sizeof(element));
    memcpy(v->data + sizeof(element), data, (sizeof(uint16_t) * v->size));
    free(data);
    v->size++;
}

void v8_push_back(v8*  v, uint8_t  element) {
    if (v->size == v->capacity) {
        v8_resize(v, v->capacity * 2);
    }
    uint8_t* data = v->data; 
    v->data = malloc(sizeof(uint8_t) * v->capacity);
    memcpy(v->data, &element, sizeof(element));
    memcpy(v->data + sizeof(element), data, (sizeof(uint8_t) * v->size));
    free(data);
    v->size++;
}

uint64_t v64_pop(v64* v) {
    uint64_t* data = v->data;
    uint64_t e = v->data[v->size - 1];
    v->data = malloc(sizeof(uint64_t) * v->capacity);
    memcpy(v->data, data, sizeof(data) - 1);
    v->size--;
    free(data);
    return e;
}

uint32_t v32_pop(v32* v) {
    uint32_t* data = v->data;
    uint32_t e = v->data[v->size - 1];
    v->data = malloc(sizeof(uint32_t) * v->capacity);
    memcpy(v->data, data, sizeof(data) - 1);
    v->size--;
    free(data);
    return e;
}

uint16_t v16_pop(v16* v) {
    uint16_t* data = v->data;
    uint16_t e = v->data[v->size - 1];
    v->data = malloc(sizeof(uint16_t) * v->capacity);
    memcpy(v->data, data, sizeof(data) - 1);
    v->size--;
    free(data);
    return e;
}

uint8_t v8_pop(v8*  v) {
    uint8_t* data = v->data;
    uint8_t e = v->data[v->size - 1];
    v->data = malloc(sizeof(uint8_t) * v->capacity);
    memcpy(v->data, data, sizeof(data) - 1);
    v->size--;
    free(data);
    return e;
}

uint64_t v64_value(v64* v, int index) {
    return v->data[index];
}

uint32_t v32_value(v32* v, int index) {
    return v->data[index];
}

uint16_t v16_value(v16* v, int index) {
    return v->data[index];
}

uint8_t v8_value(v8*  v, int index) {
    return v->data[index];
}

void v64_resize(v64* v, int new_capacity) {
    uint64_t* data = v->data;
    v->data = malloc(sizeof(uint64_t) * new_capacity);
    memcpy(v->data, data, (sizeof(uint64_t) * v->size));
    free(data);
    v->capacity = new_capacity;
}

void v32_resize(v32* v, int new_capacity) {
    uint32_t* data = v->data;
    v->data = malloc(sizeof(uint32_t) * new_capacity);
    memcpy(v->data, data, (sizeof(uint32_t) * v->size));
    free(data);
    v->capacity = new_capacity;
}

void v16_resize(v16* v, int new_capacity) {
    uint16_t* data = v->data;
    v->data = malloc(sizeof(uint16_t) * new_capacity);
    memcpy(v->data, data, (sizeof(uint16_t) * v->size));
    free(data);
    v->capacity = new_capacity;
}

void v8_resize(v8*  v, int new_capacity) {
    uint8_t* data = v->data;
    v->data = malloc(sizeof(uint8_t) * new_capacity);
    memcpy(v->data, data, (sizeof(uint8_t) * v->size));
    free(data);
    v->capacity = new_capacity;
}

void v64_append(v64* dest, v64 source) {
    int new_size = dest->size + source.size;
    uint64_t* data = malloc(sizeof(uint64_t) * new_size + 1);
    memcpy(data, dest->data, sizeof(uint64_t) * dest->size);
    memcpy(data + dest->size, source.data, sizeof(uint64_t) * source.size);
    free(dest->data);
    dest->data = data;
    dest->size = new_size;
    dest->capacity = new_size;
}

void v8_append(v8* dest, v8 source) {
    int new_size = dest->size + source.size;
    uint8_t* data = malloc(sizeof(uint8_t) * new_size + 1);
    memcpy(data, dest->data, sizeof(uint8_t) * dest->size);
    memcpy(data + dest->size, source.data, sizeof(uint8_t) * source.size);
    free(dest->data);
    dest->data = data;
    dest->size = new_size;
    dest->capacity = new_size;
}

v8 v64_convert_v8(v64 source) {
    v8 v;
    v.data = (uint8_t*)source.data;
    v.size = source.size * (sizeof(uint64_t) / sizeof(uint8_t));
    v.capacity = v.size; 
    return v;
}

v8 v8_range_of(v8* v, int start) {
    v8 new;
    uint8_t* data = malloc(v->size);
    memmove(data, &v->data[start], v->size);
    new.data = data;
    new.size = v->size;
    new.capacity = v->size;

    return new;
}

v8 v8_zeros(int amount) {
    v8 v;
    v.data = malloc(amount);
    memset(v.data, '\0', amount);
    v.size = amount;
    v.capacity = amount;
    return v;
}

void v8_push_u32(v8* v, uint32_t element) {
    if (v->size == v->capacity || v->size + 4 >= v->capacity) {
        v8_resize(v, v->capacity * 2);
    }
    v->data[v->size] = element;
    v->size += 4;
}