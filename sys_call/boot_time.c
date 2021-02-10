#include <time.h>
#include <stdio.h>

int main()
{
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);

    printf("the boot time is: %us %luns\n", tp.tv_sec, tp.tv_nsec);
}
