#include <stdio.h>

const static unsigned int x = 123;

int main() {

    int *ptr;
    printf("x = %x\n", &x);
    return 0;
}