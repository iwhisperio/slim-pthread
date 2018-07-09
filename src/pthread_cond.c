/*
 * Copyright (c) 2017-2018 iwhisper.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <windows.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "pthread_impl.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

static int _gettimeofday(struct timeval *tv)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (NULL != tv) {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        tmpres /= 10;  /*convert into microseconds*/
        /*converting file time to unix epoch*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }

    return 0;
}

int pthread_cond_init(pthread_cond_t *__cond, const pthread_condattr_t *__attr)
{
    slim_pthread_cond_t *cond = (slim_pthread_cond_t *)__cond;
    slim_pthread_condattr_t *attr = (slim_pthread_condattr_t *)__attr;
    long rc;

    if (!cond)
        return EINVAL;

    if (attr && attr->sig != _PTHREAD_CONDATTR_INIT)
        return EINVAL;

    // TODO: CHECKME!!!
    if (cond->sig != _PTHREAD_COND_INIT)
        cond->state = UNINITIALIZED;

    rc = InterlockedCompareExchange(&cond->state, INITIALIZING, UNINITIALIZED);
    if (rc == UNINITIALIZED) {
        cond->sig = _PTHREAD_COND_INIT;
        InitializeConditionVariable(&cond->condvar);
        cond->state = INITIALIZED;
    } else {
        while (cond->state != INITIALIZED)
            Sleep(0);
    }
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *__cond)
{
    slim_pthread_cond_t *cond = (slim_pthread_cond_t *)__cond;
    //long rc;

    if (!cond || cond->sig != _PTHREAD_COND_INIT)
        return EINVAL;

    // rc = InterlockedCompareExchange(&cond->state, UNINITIALIZED, INITIALIZED);
    // Nothing to do!

    memset(cond, 0, sizeof(pthread_cond_t));
    return 0;
}

int pthread_cond_broadcast(pthread_cond_t *__cond)
{
    slim_pthread_cond_t *cond = (slim_pthread_cond_t *)__cond;

    // Do not check the state field
    if (!cond || cond->sig != _PTHREAD_COND_INIT)
        return EINVAL;

    WakeAllConditionVariable(&(cond->condvar));
    return 0;
}

int pthread_cond_signal(pthread_cond_t *__cond)
{
    slim_pthread_cond_t *cond = (slim_pthread_cond_t *)__cond;

    // Do not check the state field
    if (!cond || cond->sig != _PTHREAD_COND_INIT)
        return EINVAL;

    WakeConditionVariable(&(cond->condvar));
    return 0;
}

int pthread_cond_timedwait(pthread_cond_t *__cond, pthread_mutex_t *__mutex,
    const struct timespec *abstime)
{
    slim_pthread_cond_t *cond = (slim_pthread_cond_t *)__cond;
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;
    struct timeval now;
    BOOL rc;

    long milliseconds;

    if (!cond || cond->sig != _PTHREAD_COND_INIT ||
            !mutex || mutex->sig != _PTHREAD_MUTEX_INIT ||
            mutex->state != INITIALIZED || !abstime)
        return EINVAL;

    _gettimeofday(&now);

    milliseconds = (long)((abstime->tv_sec - now.tv_sec) * 1000) +
        (long)((((abstime->tv_nsec + 500) / 1000) - now.tv_usec) / 1000);

    if (milliseconds < 0)
        milliseconds = 0;

    rc = SleepConditionVariableCS(&(cond->condvar), &(mutex->cs), milliseconds);
    return rc ? 0 : ETIMEDOUT;
}

int pthread_cond_wait(pthread_cond_t *__cond, pthread_mutex_t *__mutex)
{
    slim_pthread_cond_t *cond = (slim_pthread_cond_t *)__cond;
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;

    BOOL rc;
    if (!cond || cond->sig != _PTHREAD_COND_INIT
              || !mutex || mutex->sig != _PTHREAD_MUTEX_INIT
              || mutex->state != INITIALIZED)
        return EINVAL;

    rc = SleepConditionVariableCS(&(cond->condvar), &(mutex->cs), INFINITE);
    assert(rc == TRUE);
    return 0;
}

int pthread_condattr_init(pthread_condattr_t *__attr)
{
    slim_pthread_condattr_t *attr = (slim_pthread_condattr_t *)__attr;

    attr->sig = _PTHREAD_CONDATTR_INIT;
    attr->shared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *__attr)
{
    if (!__attr)
        return EINVAL;

    memset(__attr, 0, sizeof(pthread_condattr_t));
    return 0;
}

int pthread_condattr_getpshared(const pthread_condattr_t *__attr, int *shared)
{
    slim_pthread_condattr_t *attr = (slim_pthread_condattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_CONDATTR_INIT || !shared)
        return EINVAL;

    *shared = attr->shared;
    return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *__attr, int shared)
{
    slim_pthread_condattr_t *attr = (slim_pthread_condattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_CONDATTR_INIT ||
            (shared != PTHREAD_PROCESS_SHARED &&
            shared != PTHREAD_PROCESS_PRIVATE))
        return EINVAL;

    attr->shared = shared;
    return 0;
}
