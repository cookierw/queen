#include <assert.h>
#include <stdio.h>
#include "../src/vector.h"

void run_vector_tests();
void test_v64();

int main(int argc, char const *argv[])
{
    run_vector_tests();
    printf("SUCCESS!\n");

    return 0;
}

void run_vector_tests() {
    test_v64();
}

void test_v64() {
    v64 v = v64_new(1);

    v64_push(&v, 1);
    assert(v.size == 1);
    
    v64_push(&v, 2);
    assert(v64_value(&v, 1) == 2);
    
    int i = v64_pop(&v);
    assert(i == 2);
    assert(v.size == 1);
    
    v64_push_back(&v, 2);
    assert(v.size == 2);
    assert(v64_value(&v, 0) == 2);
}
