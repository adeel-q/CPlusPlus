#include <stddef.h>
#include <stdio.h>


void * memcopy2(void * dest, void * src, size_t n)
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




int main(void)
{
    unsigned int arr1[10] = {2, 4, 6, 8, 10, 12};
    unsigned int arr2[10] = {1, 1, 1, 1, 1, 1};

    // copy first 4 elements of arr2 into arr1
    unsigned int * retPtr;
    retPtr = memcopy2((void*)&arr1, (void*)&arr2, 4*(sizeof(unsigned int)));

    printf("Result array [");
    int i = 0;
    for (i = 0; i < 10-1; i++)
    {
        printf(" %lu,", arr1[i]);
    }
    printf( " %lu ]\n", arr1[i]);

    return 0;
}