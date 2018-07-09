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

static
BOOL CALLBACK init_wrapper(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
{
    void(*init_routine)(void) = (void(*)(void))Parameter;

    assert(init_routine);
    init_routine();
    return TRUE;
}

int pthread_once(pthread_once_t *once_control, void(*init_routine)(void))
{
    BOOL rc;

    if (!once_control || !init_routine)
        return EINVAL;

    rc = InitOnceExecuteOnce(once_control, init_wrapper, init_routine, NULL);
    assert(rc == TRUE);
    return 0;
}
