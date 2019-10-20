/*
 * simple circular string buffer library
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CIRCBUF_SPACE_LEFT(__empty, __ce, __cs, __size) (__empty) ? __size: \
    ((__size + __cs - __ce) % __size)

#ifndef CIRCBUF_ACQUIRE
#define CIRCBUF_ACQUIRE() {}
#endif

#ifndef CIRCBUF_RELEASE
#define CIRCBUF_RELEASE() {}
#endif

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
