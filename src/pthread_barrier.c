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
#include <assert.h>

#include "pthread_impl.h"

int pthread_barrier_init(pthread_barrier_t *__barrier,
        const pthread_barrierattr_t *__attr, unsigned int count)
{
    slim_pthread_barrier_t *barrier = (slim_pthread_barrier_t *)__barrier;
    slim_pthread_barrierattr_t *attr = (slim_pthread_barrierattr_t *)__attr;
    long rc;

    if (!barrier || count == 0)
        return EINVAL;

    if (attr && attr->sig != _PTHREAD_BARRIERATTR_INIT)
        return EINVAL;

    // TODO: CHECKME!!!
    if (barrier->sig != _PTHREAD_BARRIER_INIT)
        barrier->state = UNINITIALIZED;

    rc = InterlockedCompareExchange(&(barrier->state),
            INITIALIZING, UNINITIALIZED);
    if (rc == UNINITIALIZED) {
        barrier->sig = _PTHREAD_BARRIER_INIT;
        InitializeSynchronizationBarrier(&barrier->syncbar, count, -1);
        barrier->state = INITIALIZED;
    }
    else {
        while (barrier->state != INITIALIZED)
            Sleep(0);
    }
    return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *__barrier)
{
    slim_pthread_barrier_t *barrier = (slim_pthread_barrier_t *)__barrier;
    long rc;

    if (!barrier || barrier->sig != _PTHREAD_BARRIER_INIT)
        return EINVAL;

    rc = InterlockedCompareExchange(&(barrier->state),
            UNINITIALIZED, INITIALIZED);
    if (rc == INITIALIZED)
        DeleteSynchronizationBarrier(&barrier->syncbar);

    memset(barrier, 0, sizeof(pthread_barrier_t));
    return 0;
}

int pthread_barrier_wait(pthread_barrier_t *__barrier)
{
    slim_pthread_barrier_t *barrier = (slim_pthread_barrier_t *)__barrier;
    BOOL rc;

    if (!barrier || barrier->sig != _PTHREAD_BARRIER_INIT ||
            barrier->state != INITIALIZED)
        return EINVAL;

    // TODO: CHECKME!!!
    // rc = EnterSynchronizationBarrier(&barrier->syncbar, 0);
    rc = EnterSynchronizationBarrier(&barrier->syncbar,
            SYNCHRONIZATION_BARRIER_FLAGS_NO_DELETE);
    return rc ? PTHREAD_BARRIER_SERIAL_THREAD : 0;
}

int pthread_barrierattr_init(pthread_barrierattr_t *__attr)
{
    slim_pthread_barrierattr_t *attr = (slim_pthread_barrierattr_t *)__attr;

    attr->sig = _PTHREAD_BARRIERATTR_INIT;
    attr->shared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_barrierattr_destroy(pthread_barrierattr_t *__attr)
{
    if (!__attr)
        return EINVAL;

    memset(__attr, 0, sizeof(pthread_barrierattr_t));
    return 0;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *__attr,
        int *shared)
{
    slim_pthread_barrierattr_t *attr = (slim_pthread_barrierattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_BARRIERATTR_INIT || !shared)
        return EINVAL;

    *shared = attr->shared;
    return 0;
}

int pthread_barrierattr_setpshared(pthread_barrierattr_t *__attr, int shared)
{
    slim_pthread_barrierattr_t *attr = (slim_pthread_barrierattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_BARRIERATTR_INIT ||
            (shared != PTHREAD_PROCESS_SHARED &&
            shared != PTHREAD_PROCESS_PRIVATE))
        return EINVAL;

    attr->shared = shared;
    return 0;
}
