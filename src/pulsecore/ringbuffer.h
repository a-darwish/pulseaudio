#ifndef foopulseringbufferhfoo
#define foopulseringbufferhfoo

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

typedef struct pa_ringbuffer pa_ringbuffer;

struct pa_ringbuffer {
    /* Initialize those before use */
    pa_atomic_t *count;          /* Amount of data yet to be read/consumed */
    int capacity;                /* Total ringbuffer size */
    uint8_t *memory;             /* Actual backend memory */

    /* <-- Private --> */
    int readindex, writeindex;
};

/** Inquire about the amount of data that can be read
 *
 * @count: return argument; suggested amount of data to be read
 *
 * Note that if the amount of data to be read crosses the backend
 * memory physical end, this method will suggest reading up only
 * to that end. This is explicitly done to avoid extra memory
 * copies.
 *
 * Thus to read the whole data available in the ring, run a loop
 * of pa_ringbuffer_peek() and pa_ringbuffer_drop() until the
 * returned @count is zero.
 *
 * Return a pointer to the buffer where data can be read from. */
void *pa_ringbuffer_peek(pa_ringbuffer *r, int *count);

/** Move the read index by @count bytes. Must be used after reading
 * from the buffer returned by pa_ringbuffer_peek().
 *
 * Returns true only if buffer was completely full before the drop. */
bool pa_ringbuffer_drop(pa_ringbuffer *r, int count);

/** Inquire about the amount of data that can be written
 *
 * @count: return argument; suggested amount of data to be written
 *
 * Like pa_ringbuffer_peek(), if the amount of data that can be
 * written crosses the backend memory phsyical end, this method
 * will not cycle and provide a ring emulation. Build a loop of
 * begin_write() and end_write(), until returned @count is zero,
 * to write as much data to the ring as possible.
 *
 * Return a pointer to the buffer where data can be written to. */
void *pa_ringbuffer_begin_write(pa_ringbuffer *r, int *count);

/** Move the write index by @count bytes. Must be used after
 * writing to the buffer returned by pa_ringbuffer_begin_write() */
void pa_ringbuffer_end_write(pa_ringbuffer *r, int count);

#endif
