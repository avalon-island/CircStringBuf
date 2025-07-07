/*
 * CircStringBuf -- library implements fast circular buffer for C-style
 *                  (i.e, '\0'-terminated) strings.
 *
 * Copyright © 2019 sven-hm@github
 * Copyright © 2025 Andrei Kolchugin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <circstringbuf.h>

int circstringbuf_reset(circstringbuf_t* cb)
{
    if (!cb) return CIRCBUF_ERROR;

    CIRCBUF_ACQUIRE();

    cb->current_start = 0;
    cb->current_end = 0;
    cb->empty = true;

    CIRCBUF_RELEASE();
    return CIRCBUF_OK;
}

int circstringbuf_init(circstringbuf_t* cb, char* buffer,
        const size_t buffer_size)
{
    if (!buffer)
        return CIRCBUF_ERROR;
    if (buffer_size < 2)
        return CIRCBUF_ERROR;

    cb->start = buffer;
    cb->end = buffer_size;

    if (circstringbuf_reset(cb) != CIRCBUF_OK)
        return CIRCBUF_ERROR;

    return CIRCBUF_OK;
}

int circstringbuf_push(circstringbuf_t* cb, const char* string)
{
    size_t len = strlen(string) + 1;

    if (!cb || len > cb->end)
        return CIRCBUF_ERROR;

    CIRCBUF_ACQUIRE();

    size_t space_left = CIRCBUF_SPACE_LEFT(cb->empty, cb->current_end,
            cb->current_start, cb->end);

    if (len > space_left) {
        cb->current_start = (cb->current_end + len - 1) % cb->end;
        while (*(cb->start + cb->current_start) != '\0') {
            cb->current_start = (cb->current_start + 1) % cb->end;
        }
        cb->current_start = (cb->current_start + 1) % cb->end;
    }

    if (cb->current_end + len <= cb->end) {
        strncpy(cb->start + cb->current_end, string, len - 1);
        *(cb->start + cb->current_end + len - 1) = '\0';
        cb->current_end = (cb->current_end + len) % cb->end;
    }
    else {
        strncpy(cb->start + cb->current_end, string,
                cb->end - cb->current_end);
        strncpy(cb->start, string + (cb->end - cb->current_end),
                len - cb->end + cb->current_end);
        *(cb->start + len - cb->end + cb->current_end - 1) = '\0';
        cb->current_end = len - cb->end + cb->current_end;
    }

    cb->empty = false;

    CIRCBUF_RELEASE();

    if (len > space_left) {
        return CIRCBUF_DATALOSS;
    }
    return CIRCBUF_OK;
}

int circstringbuf_filllevel(circstringbuf_t* cb)
{
    if (cb->empty) return 0;
    int flevel = (cb->end + cb->current_end - cb->current_start) % cb->end;
    if (flevel == 0) return 100;
    return flevel * 100 / cb->end;
}

int circstringbuf_pop(circstringbuf_t* cb, char* string)
{
    if (!cb)
        return CIRCBUF_ERROR;
    if (cb->empty)
        return CIRCBUF_EMPTY;

    CIRCBUF_ACQUIRE();

    size_t len = strlen(cb->start + cb->current_start);

    if (cb->current_start + len >= cb->end) {
        len = cb->end - cb->current_start + strlen(cb->start);
    }

    if (cb->current_start + len < cb->end) {
        strncpy(string, cb->start + cb->current_start, len + 1);
        cb->current_start = (cb->current_start + len + 1) % cb->end;
    }
    else {
        strncpy(string, cb->start + cb->current_start,
                cb->end - cb->current_start);
        strncpy(string + (cb->end - cb->current_start), cb->start,
                (len - cb->end + cb->current_start + 1) % cb->end);
        cb->current_start = (len - cb->end + cb->current_start + 1) % cb->end;
    }

    if (cb->current_start == cb->current_end)
        cb->empty = true;

    CIRCBUF_RELEASE();
    return CIRCBUF_OK;
}
