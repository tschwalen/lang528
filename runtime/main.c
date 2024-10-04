#include <stdio.h>
#include "runtime.h"
int main(int argc, char* argv[]) {
    printf("Main method\n");
    printf("%d\n", placeholder(44));
    return 0;
}