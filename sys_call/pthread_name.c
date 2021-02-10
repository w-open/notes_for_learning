#include <stdio.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char thread[16];
    memset(thread, 0, 16);
    snprintf(thread, 16, "wangyong");
    if (0 != prctl(PR_SET_NAME, thread))
    {
        printf("error in setting thread name!\n");
    }

    memset(thread, 0, 16);
    if(0 != prctl(PR_GET_NAME, thread))
    {
        printf("error in getting thread name!\n");
    }

    printf("the thread name is %s\n", thread);

    while(1)
    {

    }


    return 0;
}
