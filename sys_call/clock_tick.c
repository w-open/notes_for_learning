#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    printf("_sc_clk_tck = %d\n", sysconf(_SC_CLK_TCK));
}
