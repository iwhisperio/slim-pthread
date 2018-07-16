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

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <windows.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SLIM_PTHREAD_BUILD
#ifdef SLIM_PTHREAD_DYNAMIC
#define PTHREAD_API __declspec(dllexport)
#else
#define PTHREAD_API
#endif
#else
#ifdef SLIM_PTHREAD_DYNAMIC
#define PTHREAD_API __declspec(dllimport)
#else
#define PTHREAD_API
#endif
#endif

#define __PTHREAD_MUTEXATTR_SIZE__      20
#define __PTHREAD_MUTEX_SIZE__          116
#define __PTHREAD_RWLOCKATTR_SIZE__     4
#define __PTHREAD_RWLOCK_SIZE__         20
#define __PTHREAD_CONDATTR_SIZE__       4
#define __PTHREAD_COND_SIZE__           12
#define __PTHREAD_BARRIERATTR_SIZE__    4
#define __PTHREAD_BARRIER_SIZE__        36
#define __PTHREAD_ATTR_SIZE__           52
#define __PTHREAD_SIZE__                84

typedef struct opaque_pthread_mutexattr_t {
    int __sig;
    char __opaque[__PTHREAD_MUTEXATTR_SIZE__];
} pthread_mutexattr_t;

typedef struct opaque_pthread_mutex_t {
    int __sig;
    char __opaque[__PTHREAD_MUTEX_SIZE__];
} pthread_mutex_t;

typedef struct opaque_pthread_rwlockattr_t {
    int __sig;
    char __opaque[__PTHREAD_RWLOCKATTR_SIZE__];
} pthread_rwlockattr_t;

typedef struct opaque_pthread_rwlock_t {
    int __sig;
    char __opaque[__PTHREAD_RWLOCK_SIZE__];
} pthread_rwlock_t;

typedef struct opaque_pthread_condattr_t {
    int __sig;
    char __opaque[__PTHREAD_CONDATTR_SIZE__];
} pthread_condattr_t;

typedef struct opaque_pthread_cond_t {
    int __sig;
    char __opaque[__PTHREAD_COND_SIZE__];
} pthread_cond_t;

typedef struct opaque_pthread_barrierattr_t {
    int __sig;
    char __opaque[__PTHREAD_BARRIERATTR_SIZE__];
} pthread_barrierattr_t;

typedef struct opaque_pthread_barrier_t {
    int __sig;
    char __opaque[__PTHREAD_BARRIER_SIZE__];
} pthread_barrier_t;

typedef struct opaque_pthread_attr_t {
    int __sig;
    char __opaque[__PTHREAD_ATTR_SIZE__];
} pthread_attr_t;

struct __slim_pthread_cleanup_handler {
    void(*__routine)(void *);
    void *__arg;
    struct __slim_pthread_cleanup_handler *__next;
};

typedef struct opaque_pthread_t {
    int __sig;
    struct __slim_pthread_cleanup_handler *__cleanup_stack;
    char __opaque[__PTHREAD_SIZE__];
} *pthread_t;

typedef INIT_ONCE pthread_once_t;

struct __slim_pthread_key_t;
typedef struct __slim_pthread_key_t *pthread_key_t;

struct sched_param {
    int sched_priority;
};

/*
 * Scheduling policies from sched.h
 */
#define SCHED_OTHER                     0
#define SCHED_FIFO                      1
#define SCHED_RR                        2
#define SCHED_BATCH                     3
#define SCHED_IDLE                      5

#define SCHED_RESET_ON_FORK             0x40000000

/*
 * Thread attributes
 */
#define PTHREAD_CREATE_JOINABLE         1
#define PTHREAD_CREATE_DETACHED         2

#define PTHREAD_INHERIT_SCHED           1
#define PTHREAD_EXPLICIT_SCHED          2

#define PTHREAD_CANCEL_ENABLE           0x01  /* Cancel takes place at next cancellation point */
#define PTHREAD_CANCEL_DISABLE          0x00  /* Cancel postponed */

#define PTHREAD_CANCEL_DEFERRED         0x00  /* Cancel waits until cancellation point */
#define PTHREAD_CANCEL_ASYNCHRONOUS     0x01  /* Cancel occurs immediately */

/* Value returned from pthread_join() when a thread is canceled */
#define PTHREAD_CANCELED                ((void *)-1)

/* We only support PTHREAD_SCOPE_SYSTEM */
#define PTHREAD_SCOPE_SYSTEM            1
#define PTHREAD_SCOPE_PROCESS           2

/*
 * Process shared attribute
 */
#define PTHREAD_PROCESS_SHARED          1
#define PTHREAD_PROCESS_PRIVATE         2

/*
 * Mutex protocol attributes
 */
#define PTHREAD_PRIO_NONE               0
#define PTHREAD_PRIO_INHERIT            1
#define PTHREAD_PRIO_PROTECT            2

/*
 * Mutex type attributes
 */
#define PTHREAD_MUTEX_NORMAL            0
#define PTHREAD_MUTEX_ERRORCHECK        1
#define PTHREAD_MUTEX_RECURSIVE         2
#define PTHREAD_MUTEX_DEFAULT           PTHREAD_MUTEX_RECURSIVE

/*
 * Object init constants
 */
#define _PTHREAD_MUTEX_INIT             0x73706D74
#define _PTHREAD_COND_INIT              0x73706376
#define _PTHREAD_RWLOCK_INIT            0x73706C6B

/*
 * Mutex variables
 */
#define PTHREAD_MUTEX_INITIALIZER       {_PTHREAD_MUTEX_INIT, {0}}
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

/*
 * RWLock variables
 */
#define PTHREAD_RWLOCK_INITIALIZER      {_PTHREAD_RWLOCK_INIT, {0}}

/*
 * Condition variables
 */
#define PTHREAD_COND_INITIALIZER        {_PTHREAD_COND_INIT, {0}}

/*
 * Barrier variables
 */
#define PTHREAD_BARRIER_SERIAL_THREAD   (-1)

 /*
  * Initialization control (once) variables
  */
#define PTHREAD_ONCE_INIT               INIT_ONCE_STATIC_INIT

/*
 * Min thread stack size on Windows
 */
#define PTHREAD_STACK_MIN               65536

 /*
  * Cancel cleanup handler management. Note, since these are implemented
  * as macros, they *MUST* occur in matched pairs!
  */

#define pthread_cleanup_push(func, val) \
    { \
        struct __slim_pthread_cleanup_handler __handler; \
        pthread_t __self = pthread_self(); \
        __handler.__routine = func; \
        __handler.__arg = val; \
        __handler.__next = __self->__cleanup_stack; \
        __self->__cleanup_stack = &__handler;

#define pthread_cleanup_pop(execute) \
        /* Note: 'handler' must be in this same lexical context! */ \
        __self->__cleanup_stack = __handler.__next; \
        if (execute) (__handler.__routine)(__handler.__arg); \
    }

PTHREAD_API
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);

PTHREAD_API
int pthread_mutex_destroy(pthread_mutex_t *mutex);

PTHREAD_API
int pthread_mutex_lock(pthread_mutex_t *mutex);

PTHREAD_API
int pthread_mutex_trylock(pthread_mutex_t *mutex);

PTHREAD_API
int pthread_mutex_unlock(pthread_mutex_t *mutex);

PTHREAD_API
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex,
        int *prioceiling);

PTHREAD_API
int pthread_mutex_setprioceiling(pthread_mutex_t *mutex,
        int prioceiling, int *old_ceiling);

PTHREAD_API
int pthread_mutexattr_init(pthread_mutexattr_t *attr);

PTHREAD_API
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

PTHREAD_API
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr,
        int *prioceiling);

PTHREAD_API
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr,
        int prioceiling);

PTHREAD_API
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
        int *protocol);

PTHREAD_API
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);

PTHREAD_API
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *shared);

PTHREAD_API
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int shared);

PTHREAD_API
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type);

PTHREAD_API
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);

PTHREAD_API
int pthread_rwlock_destroy(pthread_rwlock_t *lock);

PTHREAD_API
int pthread_rwlock_init(pthread_rwlock_t *lock,
        const pthread_rwlockattr_t *attr);

PTHREAD_API
int pthread_rwlock_rdlock(pthread_rwlock_t *lock);

PTHREAD_API
int pthread_rwlock_tryrdlock(pthread_rwlock_t *lock);

PTHREAD_API
int pthread_rwlock_trywrlock(pthread_rwlock_t *lock);

PTHREAD_API
int pthread_rwlock_wrlock(pthread_rwlock_t *lock);

PTHREAD_API
int pthread_rwlock_unlock(pthread_rwlock_t *lock);

PTHREAD_API
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);

PTHREAD_API
int pthread_rwlockattr_init(pthread_rwlockattr_t *);

PTHREAD_API
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr,
        int *shared);

PTHREAD_API
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int shared);

PTHREAD_API
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

PTHREAD_API
int pthread_cond_destroy(pthread_cond_t *cond);

PTHREAD_API
int pthread_cond_broadcast(pthread_cond_t *cond);

PTHREAD_API
int pthread_cond_signal(pthread_cond_t *cond);

PTHREAD_API
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
        const struct timespec *abstime);

PTHREAD_API
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

PTHREAD_API
int pthread_condattr_init(pthread_condattr_t *attr);

PTHREAD_API
int pthread_condattr_destroy(pthread_condattr_t *attr);

PTHREAD_API
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *shared);

PTHREAD_API
int pthread_condattr_setpshared(pthread_condattr_t *attr, int shared);

PTHREAD_API
int pthread_barrier_init(pthread_barrier_t *barrier,
        const pthread_barrierattr_t *attr, unsigned int count);

PTHREAD_API
int pthread_barrier_destroy(pthread_barrier_t *barrier);

PTHREAD_API
int pthread_barrier_wait(pthread_barrier_t *barrier);

PTHREAD_API
int pthread_barrierattr_init(pthread_barrierattr_t *attr);

PTHREAD_API
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);

PTHREAD_API
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr,
        int *shared);

PTHREAD_API
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr,
        int shared);

PTHREAD_API
int pthread_once(pthread_once_t *once_control, void(*init_routine)(void));

PTHREAD_API
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine)(void *), void *arg);

PTHREAD_API
int pthread_detach(pthread_t thread);

PTHREAD_API
int pthread_equal(pthread_t t1, pthread_t t2);

PTHREAD_API
void pthread_exit(void *value_ptr);

PTHREAD_API
int pthread_getconcurrency(void);

PTHREAD_API
int pthread_getschedparam(pthread_t thread, int *policy,
        struct sched_param *param);

PTHREAD_API
int pthread_setschedparam(pthread_t thread, int policy,
        const struct sched_param *param);

PTHREAD_API
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

PTHREAD_API
int pthread_key_delete(pthread_key_t key);

PTHREAD_API
void* pthread_getspecific(pthread_key_t);

PTHREAD_API
int pthread_setspecific(pthread_key_t , const void *value);

PTHREAD_API
int pthread_join(pthread_t thread, void **value_ptr);

PTHREAD_API
pthread_t pthread_self(void);

PTHREAD_API
int pthread_setcancelstate(int state, int *oldstate);

PTHREAD_API
int pthread_setcanceltype(int type, int *oldtype);

PTHREAD_API
void pthread_testcancel(void);

PTHREAD_API
int pthread_cancel(pthread_t thread);

PTHREAD_API
int pthread_setconcurrency(int level);

PTHREAD_API
int pthread_kill(pthread_t thread, int sig);

PTHREAD_API
int pthread_attr_init(pthread_attr_t *attr);

PTHREAD_API
int pthread_attr_destroy(pthread_attr_t *attr);

PTHREAD_API
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

PTHREAD_API
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

PTHREAD_API
int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize);

PTHREAD_API
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);

PTHREAD_API
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);

PTHREAD_API
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);

PTHREAD_API
int pthread_attr_getschedparam(const pthread_attr_t *attr,
        struct sched_param *param);

PTHREAD_API
int pthread_attr_setschedparam(pthread_attr_t *attr,
        const struct sched_param *param);

PTHREAD_API
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy);

PTHREAD_API
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);

PTHREAD_API
int pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope);

PTHREAD_API
int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope);

PTHREAD_API
int pthread_attr_getstack(const pthread_attr_t *attr,
        void **stackaddr, size_t *stacksize);

PTHREAD_API
int pthread_attr_setstack(pthread_attr_t *attr,
        void *stackaddr, size_t stacksize);

PTHREAD_API
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr);

PTHREAD_API
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);

PTHREAD_API
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize);

PTHREAD_API
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);

#ifndef SLIM_PTHREAD_DYNAMIC
BOOL pthead_module_main(
        HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PTHREAD_H__ */
