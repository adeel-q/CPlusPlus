#include <stddef.h>
#include <stdio.h>


void * memcpy2(void * dest, void * src, size_t n)
{
    // Cast input to uint8_t*ptr
    char * destPtr = (char*)dest;
    char * srcPtr = (char*)src;
    for (int i = 0; i < n; i++)
    {
        *destPtr = *srcPtr;
        destPtr++;
        srcPtr++;
    }
    return src;
}

void * memcpy_overlap(void * dest, void * src, size_t n)
{
    char temp[1024]; // <-- make some huge buffer
    char * destPtr = (char*)dest;
    char * srcPtr = (char*)src;
    for (int i =0; i <n; i++)
    {
        temp[i] = srcPtr[i];
    }
    for (int i =0; i <n; i++)
    {
        destPtr[i] = temp[i];
    }
    return dest;
}

int main(void)
{
    unsigned int arr1[10] = {2, 4, 6, 8, 10, 12};
    unsigned int arr2[10] = {1, 1, 1, 1, 1, 1};

    // copy first 4 elements of arr2 into arr1
    unsigned int * retPtr;

    printf("Source array [");
    int i = 0;
    for (i = 0; i < 10-1; i++)
    {
        printf(" %lu,", arr1[i]);
    }
    printf( " %lu ]\n", arr1[i]);

    retPtr = (unsigned int*)memcpy_overlap((void*)&arr1[1], (void*)&arr1[0], 3*(sizeof(unsigned int)));

    printf("Result array [");
    i = 0;
    for (i = 0; i < 10-1; i++)
    {
        printf(" %lu,", arr1[i]);
    }
    printf( " %lu ]\n", arr1[i]);

    return 0;
}