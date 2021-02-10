#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{

    char strtime[20];
    time_t now_time;
    struct tm *gm_time;

    memset(strtime, 0, sizeof(char)*20);
    time(&now_time);
    gm_time = gmtime(&now_time);
    gm_time->tm_hour = (gm_time->tm_hour + 8)%24;
    strftime(strtime, 20, "%Y-%m-%d %H:%M:%S", gm_time);
    printf("%s\n", strtime);

    return 0;
}
