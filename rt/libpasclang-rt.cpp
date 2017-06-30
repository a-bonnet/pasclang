#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" void* pasclang_alloc(int32_t size, unsigned char type)
{
    switch(type)
    {
        case 1: // Boolean
            return calloc(size, sizeof(char));
        case 2: // Integer
            return calloc(size, sizeof(int32_t));
        case 3: // Pointer
            return calloc(size, sizeof(char*));
        default:
            return NULL;
    }
}

extern "C" int readln()
{
    int input;
    scanf("%d", &input);
    return input;
}

extern "C" void write(int output)
{
    printf("%d", output);
}

extern "C" void writeln(int output)
{
    printf("%d\n", output);
}

