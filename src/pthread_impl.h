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

#ifndef __PTHREAD_IMPL_H__
#define __PTHREAD_IMPL_H__

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNINITIALIZED                   0
#define INITIALIZING                    1
#define INITIALIZED                     2

#define _PTHREAD_MUTEXATTR_INIT         0x73706D61
#define _PTHREAD_CONDATTR_INIT          0x73706361
#define _PTHREAD_RWLOCKATTR_INIT        0x73706C61
#define _PTHREAD_BARRIERATTR_INIT       0x73706261
#define _PTHREAD_ATTR_INIT              0x73707461

#define _PTHREAD_BARRIER_INIT           0x73706272
#define _PTHREAD_INIT                   0x73707468

#define _PTHREAD_KEY_INIT               0x73706B79

typedef struct _slim_pthread_mutexattr_t {
    int sig;
    int prioceiling;
    int protocol;
    int shared;
    int type;
} slim_pthread_mutexattr_t;

typedef struct _slim_pthread_mutex_t {
    int sig;
    long state;
    int prioceiling;
    CRITICAL_SECTION cs;
} slim_pthread_mutex_t;

typedef struct _slim_pthread_rwlockattr_t {
    int sig;
    int shared;
} slim_pthread_rwlockattr_t;

typedef struct _slim_pthread_rwlock_t {
    int sig;
    int state;
    DWORD rwstate;
    SRWLOCK srwlock;
} slim_pthread_rwlock_t;

typedef struct _slim_pthread_condattr_t {
    int sig;
    int shared;
} slim_pthread_condattr_t;

typedef struct _slim_pthread_cond_t {
    int sig;
    int state;
    CONDITION_VARIABLE condvar;
} slim_pthread_cond_t;

typedef struct _slim_pthread_barrierattr_t {
    int sig;
    int shared;
} slim_pthread_barrierattr_t;

typedef struct _slim_pthread_barrier_t {
    int sig;
    int state;
    SYNCHRONIZATION_BARRIER syncbar;
} slim_pthread_barrier_t;

typedef struct _slim_pthread_attr_t {
    int sig;
    void *stackaddr;
    size_t stacksize;
    size_t guardsize;
    int detachstate;
    int inheritsched;
    int schedpolicy;
    int schedpriority;
    int contentionscope;
} slim_pthread_attr_t;

typedef struct _slim_pthread_t {
    int sig;
    struct __slim_pthread_cleanup_handler *cleanup_stack;
    HANDLE handle;
    DWORD id;
    bool detached;
    int cancelstate;
    int canceltype;
    bool canceled;
    int schedpolicy;
    int schedpriority;
    int concurrency;
    bool normal_exit;
    void *(*start_routine)(void *);
    void *start_arg;
    void *exit_value_ptr;
} *slim_pthread_t;

static_assert(sizeof(pthread_mutexattr_t) >= sizeof(slim_pthread_mutexattr_t),
              "Size of pthread mutex attr miss match");

static_assert(sizeof(pthread_mutex_t) >= sizeof(slim_pthread_mutex_t),
              "Size of pthread mutex miss match");

static_assert(sizeof(pthread_rwlockattr_t) >= sizeof(slim_pthread_rwlockattr_t),
              "Size of pthread rwlock attr miss match");

static_assert(sizeof(pthread_rwlock_t) >= sizeof(slim_pthread_rwlock_t),
              "Size of pthread rwlock miss match");

static_assert(sizeof(pthread_condattr_t) >= sizeof(slim_pthread_condattr_t),
              "Size of pthread cond attr miss match");

static_assert(sizeof(pthread_cond_t) >= sizeof(slim_pthread_cond_t),
              "Size of pthread cond miss match");

static_assert(sizeof(pthread_barrierattr_t) >= sizeof(slim_pthread_barrierattr_t),
              "Size of pthread barrier attr miss match");

static_assert(sizeof(pthread_barrier_t) >= sizeof(slim_pthread_barrier_t),
              "Size of pthread barrier miss match");

static_assert(sizeof(pthread_attr_t) >= sizeof(slim_pthread_attr_t),
              "Size of pthread attr miss match");

static_assert(sizeof(struct opaque_pthread_t) >= sizeof(struct _slim_pthread_t),
              "Size of pthread miss match");

typedef struct __slim_pthread_key_t {
    int sig;
    DWORD slot;
    void (* destructor)(void *);
    struct __slim_pthread_key_t *prev;
    struct __slim_pthread_key_t *next;
} slim_pthread_key_t;

void slim_pthread_cleanup(void);
void slim_pthread_keys_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* __PTHREAD_IMPL_H__ */
