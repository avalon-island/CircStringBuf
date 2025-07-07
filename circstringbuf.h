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

#pragma once

/* Rudimentary form of configuration */
#if defined(HAVE_CONFIG_H)
#	include "config.h"
#else /* defined(HAVE_CONFIG_H) */
#	if __has_include("config.h")
#		include "config.h"
#	endif /* __has_include("config.h") */
#endif /* defined(HAVE_CONFIG_H) */

#include <stdint.h>
#include <stdbool.h>

#define CIRCBUF_SPACE_LEFT(__empty, __ce, __cs, __size) (__empty) ? __size: \
	((__size + __cs - __ce) % __size)

/*
 * Thread-safety macros.
 *
 * If not defined, no thread-safety is provided.
 *
 */
#if !defined(CIRCBUF_ACQUIRE)
#	define CIRCBUF_ACQUIRE {}
#endif

#if !defined(CIRCBUF_RELEASE)
#	define CIRCBUF_RELEASE {}
#endif

/*
 * Status of circular string buffer library operations.
 *
 */
enum {

    CIRCBUF_OK,
    CIRCBUF_EMPTY,
    CIRCBUF_ERROR,
    CIRCBUF_DATALOSS
};

/*
 * Circular buffer control structure.
 *
 * @field char *start           - pointer to buffer start (const)
 * @field size_t end            - buffer size (const)
 * @field size_t current_start  - current position of buffer start, changes
 *                                while buffer is partially freeing by pop()
 * @field size_t current_end    - current position of buffer end
 * @field bool empty            - self-explanatory
 *
 */
typedef struct {

    char *start;
    size_t end;

    size_t current_start;
    size_t current_end;
    bool empty;
} circstringbuf_t;

/*
 * initialize string buffer
 *
 * @param circstringbuf_t *cb   - static cirbuffer object
 * @param char* buffer          - static character buffer
 * @param size_t buffer_size    - size of buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 * @mt-safety: safe
 */
int circstringbuf_init(circstringbuf_t *, char *,
	size_t);

/*
 * reset string buffer
 *
 * @param circstringbuf_t *cb   - static cirbuffer object
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 * @mt-safety: safe
 */
int circstringbuf_reset(circstringbuf_t *);

/*
 * returns fill level of buffer in percentage
 *
 * @param circstringbuf_t *cb   - static cirbuffer object
 * @return int                  - fill level
 * @mt-safety: safe
 */
int circstringbuf_filllevel(circstringbuf_t *);

/*
 * push string to buffer
 *
 * @param circstringbuf_t *cb   - static cirbuffer object
 * @param const char* string    - string that is copied to the buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 *                              - CIRCBUF_DATALOSS if buffer is full
 * @mt-safety: safe
 */
int circstringbuf_push(circstringbuf_t *, const char *);

/*
 * pop string from circular buffer
 *
 * @param circstringbuf_t *cb   - static cirbuffer object
 * @param char *string          - string where the first valid element is
 *                                copied to
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_EMPTY if buffer is empty
 *                              - CIRCBUF_ERROR
 * @mt-safety: safe
 */
int circstringbuf_pop(circstringbuf_t *, char *);

