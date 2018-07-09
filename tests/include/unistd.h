#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <windows.h>

#define sleep(s)    Sleep((s) * 1000)
#define usleep(ms)  Sleep((ms) / 1000)

#define _SC_THREAD_PROCESS_SHARED       1
#define _SC_MAPPED_FILES                2

static __inline long sysconf(int name)
{
    long rc = -1;

    switch(name) {
    case _SC_THREAD_PROCESS_SHARED:
        rc = 0;
        break;

    case _SC_MAPPED_FILES:
        rc = 0;
        break;
    }

    return rc;
}
#endif