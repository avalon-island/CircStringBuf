/*
 * simple circular string buffer header-only library
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CIRCBUF_SPACE_LEFT(__empty, __ce, __cs, __size) (__empty) ? __size: \
    ((__size + __cs - __ce) % __size)

enum {
    CIRCBUF_OK,
    CIRCBUF_EMPTY,
    CIRCBUF_ERROR,
    CIRCBUF_DATALOSS
};

/*
 *
 */
typedef struct {
    char* start;
    size_t end;

    size_t current_start;
    size_t current_end;
    bool empty;
} circstringbuf_t;


/*
 * initialize string buffer
 *
 * @param circstringbuf_t* cb   - static cirbuffer object
 * @param char* buffer          - static character buffer
 * @param size_t buffer_size    - size of buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 */
int circstringbuf_init(circstringbuf_t* cb, char* buffer,
        const size_t buffer_size);

/*
 * reset string buffer
 *
 * @param circstringbuf_t* cb   - static cirbuffer object
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 */
int circstringbuf_reset(circstringbuf_t* cb);

/*
 * returns fill level of buffer in percentage
 *
 * @param circstringbuf_t* cb   - static cirbuffer object
 * @return int                  - fill level
 */
int circstringbuf_filllevel(circstringbuf_t* cb);

/*
 * push string to buffer
 *
 * @param circstringbuf_t* cb   - static cirbuffer object
 * @param const char* string    - string that is copied to the buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 *                              - CIRCBUF_DATALOSS if buffer is full
 */
int circstringbuf_push(circstringbuf_t* cb, const char* string);

/*
 * same as circstringbuf_push, but just returns in case of CIRCBUF_DATALOSS
 *
 * @param circstringbuf_t* cb   - static cirbuffer object
 * @param const char* string    - string that is copied to the buffer
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_ERROR on error
 *                              - CIRCBUF_DATALOSS if buffer is full
 */
int circstringbuf_trypush(circstringbuf_t* cb, const char* string);

/*
 * pop string from circular buffer
 *
 * @param circstringbuf_t* cb   - static cirbuffer object
 * @param char* string          - string where the first valid element is
 *                                copied to
 * @return enum                 - CIRCBUF_OK
 *                              - CIRCBUF_EMPTY if buffer is empty
 *                              - CIRCBUF_ERROR
 */
int circstringbuf_pop(circstringbuf_t* cb, char* string);

/*********************************************************/

int circstringbuf_reset(circstringbuf_t* cb)
{
    if (!cb) return CIRCBUF_ERROR;
    cb->current_start = 0;
    cb->current_end = 0;
    cb->empty = true;
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

    size_t space_left = CIRCBUF_SPACE_LEFT(cb->empty, cb->current_end,
            cb->current_start, cb->end);

    if (cb->current_end + len < cb->end) {
        strncpy(cb->start + cb->current_end, string, len - 1);
        *(cb->start + cb->current_end + len - 1) = '\0';
        cb->current_end += len;
    }
    else {
        strncpy(cb->start + cb->current_end, string,
                cb->end - cb->current_end);
        strncpy(cb->start, string + cb->end - cb->current_end,
                len - (cb->end - cb->current_end));
        *(cb->start + len - (cb->end - cb->current_end)) = '\0';
        cb->current_end = len - (cb->end - cb->current_end);
    }

    cb->empty = false;

    if (len > space_left) {
        cb->current_start = (cb->current_start + len) % cb->end;
        return CIRCBUF_DATALOSS;
    }
    return CIRCBUF_OK;
}

int circstringbuf_trypush(circstringbuf_t* cb, const char* string)
{
    size_t len = strlen(string) + 1;

    if (!cb || len > cb->end)
        return CIRCBUF_ERROR;

    size_t space_left = CIRCBUF_SPACE_LEFT(cb->empty, cb->current_end,
            cb->current_start, cb->end);

    if (len > space_left) {
        cb->current_start = (cb->current_start + len) % cb->end;
        return CIRCBUF_DATALOSS;
    }
    return circstringbuf_push(cb, string);
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

    size_t len = strlen(cb->start + cb->current_start);

    if (cb->current_start + len == cb->end)
        len += strlen(cb->start);

    if (cb->current_start + len < cb->end) {
        strncpy(string, cb->start + cb->current_start, len + 1);
        cb->current_start += len + 1;
    }
    else {
        strncpy(string, cb->start + cb->current_start,
                cb->end - cb->current_start);
        strncpy(string + (cb->end - cb->current_start), cb->start,
                (cb->current_start + len) % cb->end);
        cb->current_start = len - (cb->end - cb->current_start) + 1;
    }
    while (!cb->current_start) {
        cb->current_start++;
    }

    if (cb->current_start == cb->current_end)
        cb->empty = true;

    return CIRCBUF_OK;
}
