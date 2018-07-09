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

int pthread_rwlock_init(pthread_rwlock_t *__lock,
        const pthread_rwlockattr_t *__attr)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    slim_pthread_rwlockattr_t *attr = (slim_pthread_rwlockattr_t *)__attr;
    long rc;

    if (!lock)
        return EINVAL;

    if (attr && attr->sig != _PTHREAD_RWLOCKATTR_INIT)
        return EINVAL;

    // TODO: CHECKME!!!
    if (lock->sig != _PTHREAD_RWLOCK_INIT)
        lock->state = UNINITIALIZED;

    rc = InterlockedCompareExchange(&lock->state, INITIALIZING, UNINITIALIZED);
    if (rc == UNINITIALIZED) {
        lock->rwstate = TlsAlloc();
        if (lock->rwstate == TLS_OUT_OF_INDEXES) {
            lock->state = UNINITIALIZED;
            return EAGAIN;
        }

        lock->sig = _PTHREAD_RWLOCK_INIT;
        InitializeSRWLock(&lock->srwlock);
        lock->state = INITIALIZED;
    } else {
        while (lock->state != INITIALIZED)
            Sleep(0);
    }

    return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *__lock)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    long rc;

    if (!lock || lock->sig != _PTHREAD_RWLOCK_INIT)
        return EINVAL;

    rc = InterlockedCompareExchange(&lock->state, UNINITIALIZED, INITIALIZED);
    if (rc == INITIALIZED)
        TlsFree(lock->rwstate);

    memset(lock, 0, sizeof(pthread_rwlock_t));

    return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *__lock)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    size_t rwstate;

    if (!lock || lock->sig != _PTHREAD_RWLOCK_INIT)
        return EINVAL;

    if (lock->state != INITIALIZED) {
        int rc = pthread_rwlock_init(__lock, NULL);
        if (rc != 0)
            return rc;
    }

    rwstate = (size_t)TlsGetValue(lock->rwstate);
    rwstate <<= 1;
    AcquireSRWLockShared(&lock->srwlock);
    TlsSetValue(lock->rwstate, (LPVOID)rwstate);

    return 0;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *__lock)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    size_t rwstate;

    if (!lock || lock->sig != _PTHREAD_RWLOCK_INIT)
        return EINVAL;

    if (lock->state != INITIALIZED) {
        int rc = pthread_rwlock_init(__lock, NULL);
        if (rc != 0)
            return rc;
    }

    rwstate = (size_t)TlsGetValue(lock->rwstate);
    rwstate = (rwstate << 1) | 0x01;
    AcquireSRWLockExclusive(&lock->srwlock);
    TlsSetValue(lock->rwstate, (LPVOID)rwstate);

    return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *__lock)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    size_t rwstate;
    BOOLEAN rc;

    if (!lock || lock->sig != _PTHREAD_RWLOCK_INIT)
        return EINVAL;

    if (lock->state != INITIALIZED) {
        int rc = pthread_rwlock_init(__lock, NULL);
        if (rc != 0)
            return rc;
    }

    rwstate = (size_t)TlsGetValue(lock->rwstate);
    rwstate <<= 1;
    rc = TryAcquireSRWLockShared(&lock->srwlock);
    if (rc) {
        TlsSetValue(lock->rwstate, (LPVOID)rwstate);
        return 0;
    } else
        return EBUSY;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *__lock)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    size_t rwstate;
    BOOLEAN rc;

    if (!lock || lock->sig != _PTHREAD_RWLOCK_INIT)
        return EINVAL;

    if (lock->state != INITIALIZED) {
        int rc = pthread_rwlock_init(__lock, NULL);
        if (rc != 0)
            return rc;
    }

    rwstate = (size_t)TlsGetValue(lock->rwstate);
    rwstate = (rwstate << 1) | 0x01;
    rc = TryAcquireSRWLockExclusive(&lock->srwlock);
    if (rc) {
        TlsSetValue(lock->rwstate, (LPVOID)rwstate);
        return 0;
    } else
        return EBUSY;
}

int pthread_rwlock_unlock(pthread_rwlock_t *__lock)
{
    slim_pthread_rwlock_t *lock = (slim_pthread_rwlock_t *)__lock;
    size_t rwstate;

    if (!lock || lock->sig != _PTHREAD_RWLOCK_INIT ||
            lock->state != INITIALIZED)
        return EINVAL;

    rwstate = (size_t)TlsGetValue(lock->rwstate);
    if (rwstate & 0x01)
        ReleaseSRWLockExclusive(&lock->srwlock);
    else
        ReleaseSRWLockShared(&lock->srwlock);

    rwstate >>= 1;
    TlsSetValue(lock->rwstate, (LPVOID)rwstate);

    return 0;
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *__attr)
{
    slim_pthread_rwlockattr_t *attr = (slim_pthread_rwlockattr_t *)__attr;

    attr->sig = _PTHREAD_RWLOCKATTR_INIT;
    attr->shared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *__attr)
{
    if (!__attr)
        return EINVAL;

    memset(__attr, 0, sizeof(pthread_rwlockattr_t));
    return 0;
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *__attr,
        int *shared)
{
    slim_pthread_rwlockattr_t *attr = (slim_pthread_rwlockattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_RWLOCKATTR_INIT || !shared)
        return EINVAL;

    *shared = attr->shared;
    return 0;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *__attr, int shared)
{
    slim_pthread_rwlockattr_t *attr = (slim_pthread_rwlockattr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_RWLOCKATTR_INIT ||
            (shared != PTHREAD_PROCESS_SHARED &&
            shared != PTHREAD_PROCESS_PRIVATE))
        return EINVAL;

    attr->shared = shared;
    return 0;
}
