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

#include "pthread_impl.h"

int pthread_mutex_init(pthread_mutex_t *__mutex,
        const pthread_mutexattr_t *__attr)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;
    long rc;

    if (!mutex)
        return EINVAL;

    if (attr && attr->sig != _PTHREAD_MUTEXATTR_INIT)
        return EINVAL;

    // TODO: CHECKME!!!
    if (mutex->sig != _PTHREAD_MUTEX_INIT)
        mutex->state = UNINITIALIZED;

    rc = InterlockedCompareExchange(&mutex->state, INITIALIZING, UNINITIALIZED);
    if (rc == UNINITIALIZED) {
        if (attr)
            mutex->prioceiling = attr->prioceiling;
        else
            mutex->prioceiling = 0;

        mutex->sig = _PTHREAD_MUTEX_INIT;
        InitializeCriticalSection(&mutex->cs);
        mutex->state = INITIALIZED;
    }
    else {
        while (mutex->state != INITIALIZED)
            Sleep(0);
    }

    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *__mutex)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;
    long rc;

    if (!mutex || mutex->sig != _PTHREAD_MUTEX_INIT)
        return EINVAL;

    rc = InterlockedCompareExchange(&mutex->state, UNINITIALIZED, INITIALIZED);
    if (rc == INITIALIZED)
        DeleteCriticalSection(&mutex->cs);

    memset(mutex, 0, sizeof(pthread_mutex_t));

    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *__mutex)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;

    if (!mutex || mutex->sig != _PTHREAD_MUTEX_INIT)
        return EINVAL;

    if (mutex->state != INITIALIZED) {
        int rc = pthread_mutex_init(__mutex, NULL);
        if (rc != 0)
            return rc;
    }

    EnterCriticalSection(&mutex->cs);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *__mutex)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;
    BOOL rc;

    if (!mutex || mutex->sig != _PTHREAD_MUTEX_INIT)
        return EINVAL;

    if (mutex->state != INITIALIZED) {
        int rc = pthread_mutex_init(__mutex, NULL);
        if (rc != 0)
            return rc;
    }

    rc = TryEnterCriticalSection(&mutex->cs);
    return rc ? 0 : EBUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *__mutex)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;

    if (!mutex || mutex->sig != _PTHREAD_MUTEX_INIT ||
            mutex->state != INITIALIZED)
        return EINVAL;

    LeaveCriticalSection(&mutex->cs);
    return 0;
}

int pthread_mutex_getprioceiling(const pthread_mutex_t *__mutex,
        int *prioceiling)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;

    if (!mutex || mutex->sig != _PTHREAD_MUTEX_INIT ||
            mutex->state != INITIALIZED  || !prioceiling)
        return EINVAL;

    *prioceiling = mutex->prioceiling;
    return 0;
}

int pthread_mutex_setprioceiling(pthread_mutex_t *__mutex, int prioceiling,
        int *old_ceiling)
{
    slim_pthread_mutex_t *mutex = (slim_pthread_mutex_t *)__mutex;

    if (!mutex || mutex->sig != _PTHREAD_MUTEX_INIT ||
            mutex->state != INITIALIZED)
        return EINVAL;

    if (old_ceiling)
        *old_ceiling = mutex->prioceiling;

    mutex->prioceiling = prioceiling;
    return 0;
}

int pthread_mutexattr_init(pthread_mutexattr_t *__attr)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    attr->sig = _PTHREAD_MUTEXATTR_INIT;
    attr->prioceiling = 0;
    attr->protocol = PTHREAD_PRIO_NONE;
    attr->shared = PTHREAD_PROCESS_PRIVATE;
    // Default is recursive mode on Windows.
    attr->type = PTHREAD_MUTEX_RECURSIVE;

    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *__attr)
{
    if (!__attr)
        return EINVAL;

    memset(__attr, 0, sizeof(pthread_mutexattr_t));
    return 0;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *__attr,
        int *prioceiling)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT || !prioceiling)
        return EINVAL;

    *prioceiling = attr->prioceiling;
    return 0;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *__attr,
        int prioceiling)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT)
        return EINVAL;

    attr->prioceiling = prioceiling;
    return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *__attr,
        int *protocol)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT || !protocol)
        return EINVAL;

    *protocol = attr->protocol;
    return 0;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *__attr, int protocol)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT ||
            protocol < PTHREAD_PRIO_NONE || protocol > PTHREAD_PRIO_PROTECT)
        return EINVAL;

    attr->protocol = protocol;
    return 0;
}

int pthread_mutexattr_getpshared(const pthread_mutexattr_t *__attr, int *shared)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT || !shared)
        return EINVAL;

    *shared = attr->shared;
    return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t *__attr, int shared)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT ||
            (shared != PTHREAD_PROCESS_SHARED &&
            shared != PTHREAD_PROCESS_PRIVATE))
        return EINVAL;

    attr->shared = shared;
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *__attr, int *type)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT || !type)
        return EINVAL;

    *type = attr->type;
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *__attr, int type)
{
    slim_pthread_mutexattr_t *attr = (slim_pthread_mutexattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_MUTEXATTR_INIT ||
            type < PTHREAD_MUTEX_NORMAL || type > PTHREAD_MUTEX_RECURSIVE)
        return EINVAL;

    attr->type = type;
    return 0;
}
