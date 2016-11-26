/***
  This file is part of PulseAudio.

  Copyright 2014 David Henningsson, Canonical Ltd.

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <stdint.h>

#include <pulsecore/atomic.h>
#include "ringbuffer.h"

void *pa_ringbuffer_peek(pa_ringbuffer *r, int *count) {
    int c = pa_atomic_load(r->count);

    if (r->readindex + c > r->capacity)
        *count = r->capacity - r->readindex;
    else
        *count = c;

    return r->memory + r->readindex;
}

/* Returns true only if the buffer was completely full before the drop. */
bool pa_ringbuffer_drop(pa_ringbuffer *r, int count) {
    bool b = pa_atomic_sub(r->count, count) >= r->capacity;

    r->readindex += count;
    r->readindex %= r->capacity;

    return b;
}

void *pa_ringbuffer_begin_write(pa_ringbuffer *r, int *count) {
    int c = pa_atomic_load(r->count);

    *count = PA_MIN(r->capacity - r->writeindex, r->capacity - c);

    return r->memory + r->writeindex;
}

void pa_ringbuffer_end_write(pa_ringbuffer *r, int count) {
    pa_atomic_add(r->count, count);
    r->writeindex += count;
    r->writeindex %= r->capacity;
}
