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
#include <string.h>

#include <assert.h>

#include "pthread_impl.h"

static slim_pthread_key_t keys = {0, TLS_OUT_OF_INDEXES, NULL, &keys, &keys};
static pthread_mutex_t keys_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t current = NULL;

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    pthread_key_t new_key;

    if (!key)
        return EINVAL;

    new_key = (pthread_key_t)malloc(sizeof(slim_pthread_key_t));
    if (!new_key)
        return ENOMEM;

    new_key->slot = TlsAlloc();
    if (new_key->slot == TLS_OUT_OF_INDEXES) {
        free((void *)new_key);
        return EAGAIN;
    }

    new_key->sig = _PTHREAD_KEY_INIT;
    new_key->destructor = destructor;

    pthread_mutex_lock(&keys_lock);
    new_key->prev = keys.prev;
    new_key->next = &keys;
    keys.prev->next = new_key;
    keys.prev = new_key;
    pthread_mutex_unlock(&keys_lock);

    *key = new_key;
    return 0;
}

int pthread_key_delete(pthread_key_t key)
{
    if (!key || key->sig != _PTHREAD_KEY_INIT ||
            key->slot == TLS_OUT_OF_INDEXES)
        return EINVAL;

    pthread_mutex_lock(&keys_lock);
    if (current == key)
        current = key->prev;

    key->prev->next = key->next;
    key->next->prev = key->prev;
    key->next = NULL;
    key->prev = NULL;
    pthread_mutex_unlock(&keys_lock);

    TlsFree(key->slot);
    memset(key, 0, sizeof(slim_pthread_key_t));
    return 0;
}

void* pthread_getspecific(pthread_key_t key)
{
    if (!key || key->sig != _PTHREAD_KEY_INIT ||
            key->slot == TLS_OUT_OF_INDEXES)
        return NULL;

    return TlsGetValue(key->slot);
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    BOOL rc;

    if (!key || key->sig != _PTHREAD_KEY_INIT ||
            key->slot == TLS_OUT_OF_INDEXES)
        return EINVAL;

    rc = TlsSetValue(key->slot, (LPVOID)value);
    return rc ? 0 : EINVAL;
}

void slim_pthread_keys_cleanup(void)
{
    pthread_mutex_lock(&keys_lock);

    for (current = keys.next; current != &keys; current = current->next) {
        if (current->destructor) {
            void *value = TlsGetValue(current->slot);
            if (value) {
                current->destructor(value);
            }
        }
    }

    pthread_mutex_unlock(&keys_lock);
}
