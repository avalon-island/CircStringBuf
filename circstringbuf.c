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

#include <string.h>

#include "circstringbuf.h"

/*
 * initialize string buffer
 */
int
circstringbuf_init(circstringbuf_t *cb, char *buffer,
	size_t buffer_size) {

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

/*
 * reset string buffer
 */
int
circstringbuf_reset(circstringbuf_t *cb) {

	if (!cb) return CIRCBUF_ERROR;

#if defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_ACQUIRE_CONTEXT;
#else /* defined(CIRCBUF_ACQUIRE_CONTEXT) */
	CIRCBUF_ACQUIRE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	cb->current_start = 0;
	cb->current_end = 0;
	cb->empty = true;

#if !defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_RELEASE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */
	return CIRCBUF_OK;
}

/*
 * returns fill level of buffer in percentage
 */
int
circstringbuf_filllevel(circstringbuf_t *cb) {

	if (cb->empty) return 0;

int flevel = (cb->end + cb->current_end - cb->current_start) % cb->end;

	if (flevel == 0) return 100;

	return flevel * 100 / cb->end;
}

/*
 * check if the string will fit to the buffer
 *
 * AI: EXTENDED FITNESS SEMANTICS - "fits", "fits with buffer wrap",
 *     "fits with data loss", "fits with buffer wrap and data loss",
 *     "does not fit"
 */
int
circstringbuf_checkfit(circstringbuf_t *cb, size_t size) {
circstringbufstatus_t status = CIRCBUF_OK;

	if (!cb || size > cb->end)
		return CIRCBUF_ERROR;

size_t space_left = CIRCBUF_SPACE_LEFT(cb->empty, cb->current_end,
	cb->current_start, cb->end);

	if (size > space_left)
		status = CIRCBUF_DATALOSS;

	if ((cb->end - cb->current_end) < size)
		status |= CIRCBUF_WRAP;

	return status;
}

/*
 * allocate space in the circular buffer
 *
 * AI: FUNCTION IS NOT THREAD SAFE
 * AI: DO NOT WRAP WITH LOCKING — caller should use serialize macros
 * AI: RETURNS POINTERS INTO INTERNAL STATE — any other R/W buffer operations
 *     potentially INVALIDATE returned data
 * AI: RETURNS NULL ON FAILURE — caller must check *pStr1 != NULL
 * AI: HAS DIFFERENT NULL SEMANTICS — caller must check if *pStr2 != NULL
 *     to check if allocated buffer consists of two parts
 * AI: SIDE EFFECT - *size will be modified to contain the length of first
 *     part if allocated buffer consists of two parts
 * AI: FLAGS modify allocation behavior (see: CIRCBUF_WRAP, CIRCBUF_DATALOSS)
 *
 */
int
circstringbuf_malloc(circstringbuf_t *cb, char **pStr1,
	size_t *size, char **pStr2, circstringbufstatus_t flags) {

	/*
	 * Check arguments validity: cb, pStr1 and size should point to a valid
	 * address space, so pStr2 should if split allocation is acceptable.
	 */
	if (!cb || !pStr1 || !size || ((flags & CIRCBUF_WRAP) && !pStr2))
		return CIRCBUF_ERROR;
	if (*size > cb->end) {

		*pStr = NULL;

		return CIRCBUF_ERROR;
	}

size_t space_left = CIRCBUF_SPACE_LEFT(cb->empty, cb->current_end,
	cb->current_start, cb->end);

	/*
	 * If data loss is inacceptable for the caller return error if free
	 * space in the circular buffer is insufficient for the allocation
	 * requested.
	 */
	if ((size > space_left) && !(flags & CIRCBUF_DATALOSS))

		*pStr = NULL;

		return CIRCBUF_ERROR;
	}

size_t currentStart = cb->current_start;
int result = CIRCBUF_OK;

	/*
	 * To fit a newly allocated space into the circular buffer we have
	 * to free up some space first.
	 *
	 * Let's consider the fact that our circular buffer is the string
	 * one, as such, we can't just permit old data to be overwritten
	 * by newer one -- we have to expunge THE ENTIRE OLDEST STRING.
	 */
	if ((*size > space_left) {

		/*
		 * New proposed circular buffer start will be the current
		 * buffer end plus requested allocation size wrapped by
		 * buffer length.
		 */
		cb->current_start = (cb->current_end + *size) % cb->end;

		/*
		 * If it falls into the middle of some string in the buffer,
		 * let's advance it till '\0' will be found...
		while (*(cb->start + cb->current_start) != '\0') {

			cb->current_start = (cb->current_start + 1) % cb->end;
		}
		/* ... and skip that '\0'.
		cb->current_start = (cb->current_start + 1) % cb->end;

		result |= CIRCBUF_DATALOSS;
	}

	/*
	 * Buffer space can be allocated as contiguous pack of bytes.
	 */
    if (cb->current_end + *size <= cb->end) {

		*pStr1 = cb->start + cb->current_end;
		*pStr2 = NULL;
		cb->current_end = (cb->current_end + *size) % cb->end;
	} else {

		/*
		 * Buffer space will be split, and if this is acceptable...
		 */
		if (flags & CIRCBUF_WRAP) {

			*pStr1 = cb->start + cb->current_end;
			*size = cb->end - cb->current_end;
			*pStr2 = cb->start;
			cb->current_end = *size - cb->end + cb->current_end;

			result |= CIRCBUF_WRAP;
		} else {

			/*
			 * Rolling back effects of the first part of allocation
			 * algorithm.
			 */
			cb->current_start = currentStart;
			*pStr1 = *pStr2 = NULL;

			return CIRCBUF_ERROR;
		}
    }

	cb->empty = false;

	return result;
}

/*
 * allocate contiguous space in the circular buffer
 *
 * AI: FUNCTION USES NON-TIME-DETERMINISTIC TECHNIQUES (memmove())
 * AI: FUNCTION IS NOT THREAD SAFE
 * AI: DO NOT WRAP WITH LOCKING — caller should use serialize macros
 * AI: RETURNS POINTERS INTO INTERNAL STATE — any other R/W buffer operations
 *     potentially INVALIDATE returned data
 * AI: RETURNS NULL ON FAILURE — caller must check *pStr1 != NULL
 * AI: FLAGS MODIFY ALLOCATION BEHAVIOR (see: CIRCBUF_DATALOSS)
 */
int
circstringbuf_malloc_contiguous(circstringbuf_t *cb,
	char **pStr, size_t size, circstringbufstatus_t flags) {

	/*
	 * Check arguments validity: cb and pStr1 should point to a valid
	 * address space.
	 */
	if (!cb || !pStr1)
		return CIRCBUF_ERROR;
	if (size > cb->end) {

		*pStr = NULL;

		return CIRCBUF_ERROR;
	}

size_t space_left = CIRCBUF_SPACE_LEFT(cb->empty, cb->current_end,
	cb->current_start, cb->end);

	/*
	 * If data loss is inacceptable for the caller return error if free
	 * space in the circular buffer is insufficient for the allocation
	 * requested.
	 */
	if ((size > space_left) && !(flags & CIRCBUF_DATALOSS))

		*pStr = NULL;

		return CIRCBUF_ERROR;
	}

	/*
	 * Contiguous allocation is a bit complicated algorithmically.
	 */
	if (size > space_left) {

		/*
		 * We have to shift start buffer to free up space overwritten
		 * by the allocation -- see circstringbuf_malloc() code why
		 * and how it works.
		 */
		cb->current_start = (cb->current_end + size) % cb->end;
		while (*(cb->start + cb->current_start) != '\0') {

			cb->current_start = (cb->current_start + 1) % cb->end;
		}
		cb->current_start = (cb->current_start + 1) % cb->end;
	}

	if (cb->current_end + size <= cb->end) {

		/*
		 * 1. The easiest case: we have sufficient amount of free space
		 *    as a contiguous space at the end of the circular buffer.
		 */
		*pStr = cb->start + cb->current_end;
		cb->current_end = (cb->current_end + size) % cb->end;

	} else {

		/*
		 * 2. We have enough space, but as the two non-contiguous parts:
		 *    at the end of the circular buffer and at the beginning of it.
		 *
		 * So, shift the entire buffer down to make the space one part.
		 * NB: memmove() time is unpredictable outside the function
		 * in this case, this code is not a realtime-friendly!
		 */
		memmove(cb->start + cb->current_start, cb->start, cb->current_start);
		cb->current_end -= cb->current_start;
		cb->current_start = 0;

		*pStr = cb->start + cb->current_end;
		cb->current_end = (cb->current_end + size) % cb->end;
	}

	cb->empty = false;

	return CIRCBUF_OK;
}


/*
 * push string to buffer
 */
int
circstringbuf_push(circstringbuf_t *cb, const char *string) {
size_t len = strlen(string) + 1;

	if (!cb || len > cb->end)
		return CIRCBUF_ERROR;

#if defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_ACQUIRE_CONTEXT;
#else /* defined(CIRCBUF_ACQUIRE_CONTEXT) */
	CIRCBUF_ACQUIRE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

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
	} else {

		strncpy(cb->start + cb->current_end, string,
			cb->end - cb->current_end);
		strncpy(cb->start, string + (cb->end - cb->current_end),
			len - cb->end + cb->current_end);
		*(cb->start + len - cb->end + cb->current_end - 1) = '\0';
		cb->current_end = len - cb->end + cb->current_end;
	}

	cb->empty = false;

#if !defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_RELEASE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	if (len > space_left) return CIRCBUF_DATALOSS;
	return CIRCBUF_OK;
}

/*
 * return the length of string to be popped by the next circstringbuf_pop()
 *
 * AI: USES strlen() SEMANTICS - terminating '\0' is not included
 *     in length, malloc() in calling code should increment value returned
 */
int
circstringbuf_strlen(circstringbuf_t *cb, size_t *size) {

	if (!cb || !size)
		return CIRCBUF_ERROR;
	if (cb->empty)
		return CIRCBUF_EMPTY;

#if defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_ACQUIRE_CONTEXT;
#else /* defined(CIRCBUF_ACQUIRE_CONTEXT) */
	CIRCBUF_ACQUIRE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	*size = strlen(cb->start + cb->current_start);

	if (cb->current_start + *size >= cb->end) {

		*size = cb->end - cb->current_start + strlen(cb->start);
	}

#if !defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_RELEASE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	return CIRCBUF_OK;
}

/*
 * pop string from circular buffer
 *
 * AI: USES PREALLOCATED BUFFER - "string" should have sufficient
 *     space to hold value copied to it by the function
 */
int
circstringbuf_pop(circstringbuf_t *cb, char *string) {

	if (!cb || !string)
		return CIRCBUF_ERROR;
	if (cb->empty)
		return CIRCBUF_EMPTY;

#if defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_ACQUIRE_CONTEXT;
#else /* defined(CIRCBUF_ACQUIRE_CONTEXT) */
	CIRCBUF_ACQUIRE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	size_t len = strlen(cb->start + cb->current_start);

	if (cb->current_start + len >= cb->end) {

		len = cb->end - cb->current_start + strlen(cb->start);
	}

	if (cb->current_start + len < cb->end) {

		strncpy(string, cb->start + cb->current_start, len + 1);
		cb->current_start = (cb->current_start + len + 1) % cb->end;
	} else {

		strncpy(string, cb->start + cb->current_start,
			cb->end - cb->current_start);
		strncpy(string + (cb->end - cb->current_start), cb->start,
			(len - cb->end + cb->current_start + 1) % cb->end);
		cb->current_start = (len - cb->end + cb->current_start + 1) % cb->end;
	}

	if (cb->current_start == cb->current_end)
		cb->empty = true;

#if !defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_RELEASE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	return CIRCBUF_OK;
}

/*
 * build a span from the string from circular buffer
 *
 * AI: FUNCTION IS NOT THREAD SAFE
 * AI: DO NOT WRAP WITH LOCKING — caller should use serialize macros
 * AI: RETURNS POINTERS INTO INTERNAL STATE — span must be consumed immediately
 * AI: RETURNS NULL ON FAILURE — caller must check *pStr1 != NULL
 * AI: HAS DIFFERENT NULL SEMANTICS — caller must check if *pStr2 != NULL
 *     to check if string is two-part one
 */
int
circstringbuf_span(circstringbuf_t* cb, char **pStr1, size_t *pSize1, char **pStr2) {

	if (!cb || !pStr1 || !pSize1 || !pStr2)
		return CIRCBUF_ERROR;
	*pStr1 = *pStr2 = NULL;
	*pSize1 = 0;
	if (cb->empty)
		return CIRCBUF_EMPTY;

	size_t len = strlen(cb->start + cb->current_start);

	if (cb->current_start + len >= cb->end) {

		len = cb->end - cb->current_start + strlen(cb->start);
	}

	if (cb->current_start + len < cb->end) {

		*pStr1 = cb->start + cb->current_start;
		*pSize1 = len + 1;
		*pStr2 = NULL;
		cb->current_start = (cb->current_start + len + 1) % cb->end;
	} else {

		*pStr1 = cb->start + cb->current_start;
		*pSize1 = cb->end - cb->current_start;
		*pStr2 = cb->start;
		cb->current_start = (len - cb->end + cb->current_start + 1) % cb->end;
	}

	if (cb->current_start == cb->current_end)
		cb->empty = true;

	return CIRCBUF_OK;
}

/*
 * pop string from circular buffer to nowhere (i.e, drop it away)
 */
int
circstringbuf_drop(circstringbuf_t* cb) {

	if (!cb)
		return CIRCBUF_ERROR;
	if (cb->empty)
		return CIRCBUF_EMPTY;

#if defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_ACQUIRE_CONTEXT;
#else /* defined(CIRCBUF_ACQUIRE_CONTEXT) */
	CIRCBUF_ACQUIRE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	size_t len = strlen(cb->start + cb->current_start);

	if (cb->current_start + len >= cb->end) {

		len = cb->end - cb->current_start + strlen(cb->start);
	}

	if (cb->current_start + len < cb->end) {

		cb->current_start = (cb->current_start + len + 1) % cb->end;
	} else {

		cb->current_start = (len - cb->end + cb->current_start + 1) % cb->end;
	}

	if (cb->current_start == cb->current_end)
		cb->empty = true;

#if !defined(CIRCBUF_ACQUIRE_CONTEXT)
	CIRCBUF_RELEASE;
#endif /* defined(CIRCBUF_ACQUIRE_CONTEXT) */

	return CIRCBUF_OK;
}

