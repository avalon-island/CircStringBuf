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
 * Status (proposed/acceptable and real) of circular string buffer library
 * operations.
 *
 */
typedef enum {

	CIRCBUF_OK = 0,
	CIRCBUF_EMPTY = -1,
	CIRCBUF_ERROR = -2,
	CIRCBUF_WRAP = 1,
	CIRCBUF_DATALOSS = 2
} circstringbufstatus_t;

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
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param char *buffer          - static character buffer
 * @param size_t buffer_size    - size of buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_init(circstringbuf_t *, char *,
	size_t);

/*
 * reset string buffer
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_reset(circstringbuf_t *);

/*
 * returns fill level of buffer in percentage
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @return int                  - fill level
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_filllevel(circstringbuf_t *);

/*
 * check if the string will fit to buffer
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param size_t *pSize         - pointer to variable holds size to check
 *                                and where maximum contiguous space size
 *                                will be stored in a case of CIRCBUF_WRAP
 * @return enum                 - CIRCBUF_OK if string will fit natively
 *                              - CIRCBUF_WRAP if string will fit with
 *                                buffer wrapping, i.e. as two
 *                                non-contiguous parts
 *                              - CIRCBUF_DATALOSS if string will not
 *                                fit into free space of the buffer,
 *                                data loss will occur
 *                              - CIRCBUF_ERROR if string will not fit
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_checkfit(circstringbuf_t *,
	size_t *);

/*
 * allocate space in the circular buffer
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param char **str1           - pointer to variable which will be set
 *                                to pointer to allocated space (or to
 *                                the first part of allocated space in a case
 *                                of circular buffer wrapping) or to NULL
 *                                if allocation is impossible
 * @param size_t *size          - pointer to variable holds the size of
 *                                the buffer to be allocated --
 *                                don't forget to +1 it to store the termi-
 *                                nating '\0'! In a case of circular buffer
 *                                wrapping it will be set to the size of
 *                                the first part of allocated space, so, store
 *                                its original value elsewhere if you are
 *                                accepting non-contiguous allocations
 * @param char **str2           - pointer to variable which will be set
 *                                to pointer to the second part of allocated
 *                                space in a case of circular buffer wrapping
 *                                or to NULL if no wrapping was occured
 * @param enum                  - CIRCBUF_WRAP if you accept non-contiguous
 *                                allocations
 *                              - CIRCBUF_DATALOSS if you accept buffer
 *                                overflow with old data loss
 * @return enum                 - CIRCBUF_OK if string will fit natively
 *                              - CIRCBUF_WRAP if string will fit with
 *                                buffer wrapping, i.e. as two
 *                                non-contiguous parts
 *                              - CIRCBUF_DATALOSS if string will not
 *                                fit into free space of the buffer,
 *                                data loss will occur
 *                              - CIRCBUF_ERROR if string will not fit
 *
 * AI: FUNCTION IS NOT THREAD SAFE
 * AI: DO NOT WRAP WITH LOCKING — caller should use serialize macros
 * AI: RETURNS POINTERS INTO INTERNAL STATE — any other R/W buffer operations
 *     potentially INVALIDATE returned data
 *
 * @mt-safety: unsafe           - circstringbuf_malloc() returns pointer
 *                                to internal buffer space, as such, any
 *                                operations on the buffer which modify its
 *                                state should be postponed while memory
 *                                allocated is accessed!
 *
 */
int circstringbuf_malloc(circstringbuf_t *, char **,
	size_t *, char **, circstringbufstatus_t);

/*
 * allocate contiguous space in the circular buffer
 * NB: circstringbuf_malloc_contiguous() will use memmove() to free up
 *     space in the circular buffer, keep in mind this if you are writing
 *     real-time application
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param char **string         - pointer to variable which will be set
 *                                to pointer to allocated space or to NULL
 *                                if allocation is impossible
 * @param size_t size           - size of the buffer to be allocated --
 *                                don't forget to +1 it to store the termi-
 *                                nating '\0'!
 * @param enum                  - CIRCBUF_DATALOSS if you accept buffer
 *                                overflow with old data loss
 * @return enum                 - CIRCBUF_OK if string will fit natively
 *                              - CIRCBUF_DATALOSS if string will not
 *                                fit into free space of the buffer,
 *                                data loss will occur
 *                              - CIRCBUF_ERROR if string will not fit
 *
 * AI: FUNCTION IS NOT THREAD SAFE
 * AI: DO NOT WRAP WITH LOCKING — caller should use serialize macros
 * AI: RETURNS POINTERS INTO INTERNAL STATE — any other R/W buffer operations
 *     potentially INVALIDATE returned data
 *
 * @mt-safety: unsafe           - circstringbuf_malloc_contiguous() returns
 *                                pointer to internal buffer space, as such,
 *                                any operations on the buffer which modify
 *                                its state should be postponed while memory
 *                                allocated is accessed!
 *
 */
int circstringbuf_malloc_contiguous(circstringbuf_t *,
	char **, size_t, circstringbufstatus_t);

/*
 * push string to buffer
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param const char *string    - string that is copied to the buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_WRAP if buffer wrapping has
 *                                occured
 *                              - CIRCBUF_ERROR on error
 *                              - CIRCBUF_DATALOSS if buffer is full
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_push(circstringbuf_t *,const char *);

/*
 * return the length of string to be popped by the next circstringbuf_pop()
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param size_t *size          - variable where the length of the first
 *                                valid element is copied to
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_EMPTY if buffer is empty
 *                              - CIRCBUF_ERROR on error
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_strlen(circstringbuf_t *, size_t *);

/*
 * pop string from circular buffer
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param char *string          - string where the first valid element is
 *                                copied to
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_EMPTY if buffer is empty
 *                              - CIRCBUF_ERROR on error
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_pop(circstringbuf_t *, char *);

/*
 * build a span from the string from circular buffer
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @param char **part1          - pointer to variable which be set to the first
 *                                part of possibly split (and NOT-'\0'-TER-
 *                                MINATED IN THIS CASE!) string
 * @param size_t *spart1        - pointer to variable which will store the size
 *                                of the first part excluding terminating '\0'
 *                                if it ever exists
 * @param char **part2          - pointer to variable which be set to the second
 *                                part of possibly split string (always '\0'-
 *                                terminated because no more than two parts
 *                                can exist), NULL otherwise
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_EMPTY if buffer is empty
 *                              - CIRCBUF_WRAP if *part2 != NULL
 *                              - CIRCBUF_ERROR on error
 *
 * AI: FUNCTION IS NOT THREAD SAFE
 * AI: DO NOT WRAP WITH LOCKING — caller should use serialize macros
 * AI: RETURNS POINTERS INTO INTERNAL STATE — span must be consumed immediately
 *
 * @mt-safety: unsafe           - circstringbuf_span() returns pointer to internal
 *                                buffer space, as such, any operations on
 *                                the buffer which modify its state should be
 *                                postponed while span is accessed!
 *
 */
int circstringbuf_span(circstringbuf_t *, char **,
	size_t *, char **);

/*
 * pop string from circular buffer to nowhere (i.e, drop it away)
 *
 * @param circstringbuf_t *cb   - static circbuffer object
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_EMPTY if buffer is empty
 *                              - CIRCBUF_ERROR on error
 *
 * @mt-safety: safe
 *
 */
int circstringbuf_drop(circstringbuf_t *);

