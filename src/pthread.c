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
#include <stdint.h>
#include <stdbool.h>
#include <process.h>
#include <assert.h>

#include "pthread_impl.h"

__declspec(thread) slim_pthread_t self = NULL;

void slim_pthread_cleanup(void)
{
    if (!self)
        return;

    assert(self->sig == _PTHREAD_INIT);

    while (self->cleanup_stack) {
        self->cleanup_stack->__routine(self->cleanup_stack->__arg);
        self->cleanup_stack = self->cleanup_stack->__next;
    }

    slim_pthread_keys_cleanup();

    self->normal_exit = 1;

    if (self->detached) {
        memset(self, 0, sizeof(struct _slim_pthread_t));
        free(self);
    }

    self = NULL;
}

static unsigned int __stdcall pthread_start_routine(void *arg)
{
    self = (slim_pthread_t)arg;
    assert(self && self->sig == _PTHREAD_INIT);

    self->exit_value_ptr = self->start_routine(self->start_arg);

    slim_pthread_cleanup();
    return 0;
}

int pthread_create(pthread_t *__thread, const pthread_attr_t *__attr,
        void *(*start_routine)(void *), void *arg)
{
    slim_pthread_attr_t attr;
    slim_pthread_t thread;
    unsigned int stacksize;

    if (!__thread || !start_routine)
        return EINVAL;

    if (__attr && __attr->__sig != _PTHREAD_ATTR_INIT)
        return EINVAL;

    thread = (slim_pthread_t)calloc(1, sizeof(struct opaque_pthread_t));
    if (!thread)
        return EAGAIN;

    if (__attr)
        attr = *(slim_pthread_attr_t *)__attr;
    else
        pthread_attr_init((pthread_attr_t *)&attr);

    thread->sig = _PTHREAD_INIT;
    thread->detached = (attr.detachstate == PTHREAD_CREATE_DETACHED);
    thread->cancelstate = PTHREAD_CANCEL_ENABLE;
    thread->canceltype = PTHREAD_CANCEL_DEFERRED;
    thread->schedpolicy = attr.schedpolicy;
    thread->schedpriority = attr.schedpriority;
    thread->start_routine = start_routine;
    thread->start_arg = arg;

    stacksize = (unsigned int)attr.stacksize;

    thread->handle = (HANDLE)_beginthreadex(NULL, stacksize,
            pthread_start_routine, (void *)thread, 0, &thread->id);
    if (thread->handle == (HANDLE)(-1L)) {
        int rc = errno;
        memset((void *)thread, 0, sizeof(struct _slim_pthread_t));
        free((void *)thread);
        return rc;
    }

    *__thread = (pthread_t)thread;
    return 0;
}

int pthread_detach(pthread_t __thread)
{
    slim_pthread_t thread = (slim_pthread_t)__thread;

    if (!thread || thread->sig != _PTHREAD_INIT)
        return EINVAL;

    if (thread->detached)
        return EINVAL;

    thread->detached = true;
    return 0;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    if (!t1 || t1->__sig != _PTHREAD_INIT || !t2 || t1->__sig != _PTHREAD_INIT)
        return 0;

    return ((slim_pthread_t)t1)->id == ((slim_pthread_t)t2)->id;
}

void pthread_exit(void *value_ptr)
{
    if (!self) {
        // Current thread not pthread compatible, ignore value_ptr
        _endthreadex(0);
    }

    self->exit_value_ptr = value_ptr;
    slim_pthread_cleanup();
    _endthreadex(0);
}

int pthread_getconcurrency(void)
{
    // Always succeeds, so return 0 for non pthread.
    if (!self)
        return 0;

    return self->concurrency;
}

int pthread_setconcurrency(int level)
{
    if (!self)
        return ENOTSUP;

    if (level < 0)
        return EINVAL;

    self->concurrency = level;
    return 0;
}

int pthread_getschedparam(pthread_t __thread, int *policy,
        struct sched_param *param)
{
    slim_pthread_t thread = (slim_pthread_t)__thread;

    if (!thread || thread->sig != _PTHREAD_INIT)
        return ESRCH;

    if (policy)
        *policy = thread->schedpolicy;

    if (param)
        param->sched_priority = thread->schedpriority;

    return 0;
}

int pthread_setschedparam(pthread_t __thread, int policy,
        const struct sched_param *param)
{
    slim_pthread_t thread = (slim_pthread_t)__thread;

    if (!thread || thread->sig != _PTHREAD_INIT)
        return ESRCH;

    if ((policy & (~SCHED_RESET_ON_FORK)) < SCHED_OTHER ||
            (policy & (~SCHED_RESET_ON_FORK)) > SCHED_IDLE)
        return EINVAL;

    if ((policy & (~SCHED_RESET_ON_FORK)) != SCHED_OTHER)
        return ENOTSUP;

    if (!param || param->sched_priority != 0)
        return EINVAL;

    thread->schedpolicy = policy;
    thread->schedpriority = param->sched_priority;

    return 0;
}

int pthread_join(pthread_t __thread, void **value_ptr)
{
    slim_pthread_t thread = (slim_pthread_t)__thread;

    if (!thread || thread->sig != _PTHREAD_INIT)
        return ESRCH;

    if (thread->detached)
        return EINVAL;

    if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0)
        return EINTR;

    if (value_ptr)
        *value_ptr = thread->exit_value_ptr;

    memset(thread, 0, sizeof(struct opaque_pthread_t));
    free(thread);
    return 0;
}

pthread_t pthread_self(void)
{
    if (!self) {
        self = (slim_pthread_t)calloc(1, sizeof(struct opaque_pthread_t));
        if (self) {
            self->sig = _PTHREAD_INIT;
            self->handle = GetCurrentThread();
            self->id = GetCurrentThreadId();
            self->detached = true;
            self->cancelstate = PTHREAD_CANCEL_ENABLE;
            self->canceltype = PTHREAD_CANCEL_DEFERRED;
            self->schedpolicy = SCHED_OTHER;
            self->schedpriority = 0;
            self->start_routine = NULL;
            self->start_arg = NULL;
        }
    }

    return (pthread_t)self;
}

PTHREAD_API
int pthread_setcancelstate(int state, int *oldstate)
{
    if (!self)
        return ENOTSUP;

    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE)
        return EINVAL;

    if (oldstate)
        *oldstate = self->cancelstate;

    self->cancelstate = state;
    return 0;
}

int pthread_setcanceltype(int type, int *oldtype)
{
    if (!self)
        return ENOTSUP;

    if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS)
        return EINVAL;

    if (type == PTHREAD_CANCEL_ASYNCHRONOUS)
        return ENOTSUP; // TODO: support asynchronous mode later.

    if (oldtype)
        *oldtype = self->canceltype;

    self->canceltype = type;
    return 0;
}

void pthread_testcancel(void)
{
    if (!self)
        return; // Not a pthread

    if (self->cancelstate == PTHREAD_CANCEL_ENABLE && self->canceled)
        pthread_exit(PTHREAD_CANCELED);
}

int pthread_cancel(pthread_t __thread)
{
    slim_pthread_t thread = (slim_pthread_t)__thread;

    if (!thread || thread->sig != _PTHREAD_INIT)
        return ESRCH;

    thread->canceled = true;
    return 0;
}

int pthread_kill(pthread_t __thread, int sig)
{
    return ENOTSUP;
}

int pthread_attr_init(pthread_attr_t *__attr)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    attr->sig = _PTHREAD_ATTR_INIT;
    attr->stackaddr = NULL;
    attr->stacksize = 0;
    attr->guardsize = 0;
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->inheritsched = PTHREAD_INHERIT_SCHED;
    attr->schedpriority = 0;
    attr->schedpolicy = SCHED_OTHER;
    attr->contentionscope = PTHREAD_SCOPE_SYSTEM;

    return 0;
}

int pthread_attr_destroy(pthread_attr_t *__attr)
{
    if (!__attr)
        return EINVAL;

    memset(__attr, 0, sizeof(pthread_attr_t));
    return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *__attr, int *detachstate)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !detachstate)
        return EINVAL;

    *detachstate = attr->detachstate;
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *__attr, int detachstate)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT ||
            (detachstate != PTHREAD_CREATE_JOINABLE &&
            detachstate != PTHREAD_CREATE_DETACHED))
        return EINVAL;

    attr->detachstate = detachstate;
    return 0;
}

int pthread_attr_getguardsize(const pthread_attr_t *__attr, size_t *guardsize)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !guardsize)
        return EINVAL;

    *guardsize = attr->guardsize;
    return 0;
}

int pthread_attr_setguardsize(pthread_attr_t *__attr, size_t guardsize)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT)
        return EINVAL;

    attr->guardsize = guardsize;
    return 0;
}

int pthread_attr_getinheritsched(const pthread_attr_t *__attr,
        int *inheritsched)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !inheritsched)
        return EINVAL;

    *inheritsched = attr->inheritsched;
    return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *__attr, int inheritsched)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT ||
            (inheritsched != PTHREAD_INHERIT_SCHED &&
            inheritsched != PTHREAD_EXPLICIT_SCHED))
        return EINVAL;

    attr->inheritsched = inheritsched;
    return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *__attr,
        struct sched_param *param)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !param)
        return EINVAL;

    param->sched_priority = attr->schedpriority;
    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *__attr,
        const struct sched_param *param)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT ||
            !param || param->sched_priority != 0)
        return EINVAL;

    attr->schedpriority = param->sched_priority;
    return 0;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *__attr, int *policy)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !policy)
        return EINVAL;

    *policy = attr->schedpolicy;
    return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *__attr, int policy)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT)
        return EINVAL;

    if ((policy & (~SCHED_RESET_ON_FORK)) < SCHED_OTHER ||
            (policy & (~SCHED_RESET_ON_FORK)) > SCHED_IDLE)
        return EINVAL;

    if ((policy & (~SCHED_RESET_ON_FORK)) != SCHED_OTHER)
        return ENOTSUP;

    attr->schedpolicy = policy;
    return 0;
}

int pthread_attr_getscope(const pthread_attr_t *__attr, int *contentionscope)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !contentionscope)
        return EINVAL;

    *contentionscope = attr->contentionscope;
    return 0;
}

int pthread_attr_setscope(pthread_attr_t *__attr, int contentionscope)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT)
        return EINVAL;

    if ((contentionscope != PTHREAD_SCOPE_SYSTEM &&
            contentionscope != PTHREAD_SCOPE_PROCESS))
        return EINVAL;

    if (contentionscope == PTHREAD_SCOPE_PROCESS)
        return ENOTSUP;

    attr->contentionscope = contentionscope;
    return 0;
}

int pthread_attr_getstack(const pthread_attr_t *__attr,
        void **stackaddr, size_t *stacksize)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT)
        return EINVAL;

    if (stackaddr)
        *stackaddr = attr->stackaddr;

    if (stacksize)
        *stacksize = attr->stacksize;

    return 0;
}

int pthread_attr_setstack(pthread_attr_t *__attr,
        void *stackaddr, size_t stacksize)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT ||
            stacksize < PTHREAD_STACK_MIN)
        return EINVAL;

    if (stackaddr)
        return ENOTSUP;

    attr->stacksize = stacksize;
    return 0;
}

int pthread_attr_getstackaddr(const pthread_attr_t *__attr, void **stackaddr)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !stackaddr)
        return EINVAL;

    *stackaddr = attr->stackaddr;
    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *__attr, void *stackaddr)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT)
        return EINVAL;

    if (stackaddr)
        return ENOTSUP;

    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *__attr, size_t *stacksize)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT || !stacksize)
        return EINVAL;

    *stacksize = attr->stacksize;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *__attr, size_t stacksize)
{
    slim_pthread_attr_t *attr = (slim_pthread_attr_t *)__attr;

    if (!attr || attr->sig != _PTHREAD_ATTR_INIT ||
            stacksize < PTHREAD_STACK_MIN)
        return EINVAL;

    attr->stacksize = stacksize;
    return 0;
}
