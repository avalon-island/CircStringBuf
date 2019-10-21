#include <stdio.h>
#include <stdint.h>
#include "unity.h"

#include <stdlib.h>
#include <time.h>

#include <circstringbuf.h>

#define BUFFER_SIZE (10240)

static char buffer[BUFFER_SIZE];

static circstringbuf_t cbuff;

void printb(int size)
{
    int ii;
    for (ii = 0; ii < size; ii++)
    {
        if (buffer[ii])
            printf("%c", buffer[ii]);
        else
            printf("-");
    }

    if (cbuff.empty)
        printf("   (empty)\n");
    else
        printf("\n");
    ii = 0;
    if (cbuff.current_start == cbuff.current_end) {
        while (ii++ < cbuff.current_start) printf(" ");
        if (cbuff.empty)
            printf("^");
        else
            printf("#");
    }
    else if (cbuff.current_start < cbuff.current_end) {
        while (ii++ < cbuff.current_start) printf(" ");
        printf("<");
        while (ii++ < cbuff.current_end) printf(".");
        printf(">");
    }
    else {
        while (ii++ < cbuff.current_end) printf(".");
        printf(">");
        while (ii++ < cbuff.current_start) printf(" ");
        printf("<");
        while (ii++ < size) printf(".");
    }
    printf("\n");
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_circstringbufpush(void)
{
    circstringbuf_init(&cbuff, buffer, 20);

    TEST_ASSERT_EQUAL(CIRCBUF_ERROR, circstringbuf_push(&cbuff,
                "12345678901234567890"));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test1"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test2"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test3"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test4"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test5"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test6"));
}

void test_circstringbufpop(void)
{
    circstringbuf_init(&cbuff, buffer, 20);
    char tmp_buf[10];

    TEST_ASSERT_EQUAL(CIRCBUF_EMPTY, circstringbuf_pop(&cbuff, tmp_buf));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test1"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test2"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test3"));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test1", tmp_buf);
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test2", tmp_buf);
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test3", tmp_buf);
    TEST_ASSERT_EQUAL(CIRCBUF_EMPTY, circstringbuf_pop(&cbuff, tmp_buf));
}

void test_circstringbufpopdataloss(void)
{
    circstringbuf_init(&cbuff, buffer, 20);
    char tmp_buf[10];

    TEST_ASSERT_EQUAL(CIRCBUF_EMPTY, circstringbuf_pop(&cbuff, tmp_buf));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test1"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test2"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test3"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test4"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test5"));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test3", tmp_buf);

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test6"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test7"));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test5", tmp_buf);
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test6", tmp_buf);
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("test7", tmp_buf);
    TEST_ASSERT_EQUAL(CIRCBUF_EMPTY, circstringbuf_pop(&cbuff, tmp_buf));
}

void test_circstringbufpushemptystring(void)
{
    circstringbuf_init(&cbuff, buffer, 20);

    char tmp_buf[10];
    for (int ii = 0; ii < 20; ii++) {
        TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, ""));
    }
    TEST_ASSERT_EQUAL(100, circstringbuf_filllevel(&cbuff));
    for (int ii = 0; ii < 5; ii++) {
        TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, ""));
    }
    TEST_ASSERT_EQUAL(100, circstringbuf_filllevel(&cbuff));
    for (int ii = 0; ii < 20; ii++) {
        TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
        TEST_ASSERT_EQUAL_STRING("", tmp_buf);
    }
    TEST_ASSERT_EQUAL(0, circstringbuf_filllevel(&cbuff));
    TEST_ASSERT_EQUAL(CIRCBUF_EMPTY, circstringbuf_pop(&cbuff, tmp_buf));
}

void test_circstringbufstuff(void)
{
    circstringbuf_init(&cbuff, buffer, 20);
    char tmp_buf[13];
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "Hallo Welt!"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
    TEST_ASSERT_EQUAL_STRING("Hallo Welt!", tmp_buf);
}

static char rand_string(char* array, int min_len, int max_len)
{
    int len = (rand() % (max_len - min_len)) + min_len;
    int ii = 0;
    while (ii < len) {
        array[ii++] = ((unsigned)rand() % 95) + 32;
    }
    array[ii] = '\0';
}

void test_circstringbufrandomstrings_1(void)
{
    /* In this test we push strings into the buffer until we reach the end,
     * i.e. we overwride some of the first strings. We store all strings for
     * comparison in `testbuffer`.
     */
    circstringbuf_init(&cbuff, buffer, BUFFER_SIZE);

    const int testbuffersize = 40 * 10 * 4;
    const int maxstringsize = 256;
    char testbuffer[testbuffersize * maxstringsize];

    srand(time(NULL));
    int count = 0;

    while (count++ < 10000) {
        char tmp_buf[maxstringsize];
        int pushindex = 0;
        int last_string_size = 0;

        // push data
        while (pushindex < testbuffersize) {
            char* current_string = &testbuffer[maxstringsize * pushindex++];
            // we avoid too short strings at the beginning
            // because otherwise searching the first matching will fail
            rand_string(current_string, pushindex < 10 ? 5 : 0, maxstringsize);
            last_string_size = strlen(current_string);

            // push data until queue is full
            int retval = circstringbuf_push(&cbuff, current_string);
            if (retval == CIRCBUF_OK)
                continue;
            else if (retval == CIRCBUF_DATALOSS)
                break;
            else
                TEST_ASSERT_TRUE(false);
        }

        // the last push has caused data loss
        // so we search the first matching string
        int ii = 0;
        TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_pop(&cbuff, tmp_buf));
        while (strcmp(&testbuffer[ii++ * maxstringsize], tmp_buf) != 0
                && ii < 10) {
        }
        // we should not have lost so many strings
        // but this test can fail
        TEST_ASSERT_TRUE(ii < 15);

        while (circstringbuf_pop(&cbuff, tmp_buf) == CIRCBUF_OK) {
            if (strcmp(&testbuffer[ii++ * maxstringsize], tmp_buf) != 0)
            {
                // in case of failure print the first few strings
                // stored in testbuffer
                printf("==================================================\n");
                for (int kk = 0; kk < 10; kk++)
                    printf("%d: (%d) %s\n", kk,
                            strlen(&testbuffer[kk * maxstringsize]),
                            &testbuffer[kk * maxstringsize]);
                printf("==================================================\n");
                TEST_ASSERT_TRUE(false);
            }
        }
        TEST_ASSERT_EQUAL(0, circstringbuf_filllevel(&cbuff));
    }
}

void test_circstringbufrandomstrings_2(void)
{
    /* In this test we push strings into the buffer until we have a specific
     * number of data losses. We store a big number of strings for comparison
     * in `testbuffer`, at least more than fit into the circ-buffer.
     */
    circstringbuf_init(&cbuff, buffer, BUFFER_SIZE);

    const unsigned int testbuffersize = 40 * 10;
    const unsigned int maxstringsize = 256;
    unsigned int count = 0;
    unsigned int pushindex = 0;
    unsigned int popindex = 0;
    char testbuffer[testbuffersize * maxstringsize];
    char testbuffer_pop[testbuffersize * maxstringsize];
    char tmp_buf[maxstringsize];

    srand(time(NULL));

    // push lots of data, until you have a number of data losses
    while (count < 1000) {
        char* current_string = &testbuffer[maxstringsize * pushindex];
        pushindex = (pushindex + 1) % testbuffersize;
        rand_string(current_string, 0, maxstringsize);

        int retval = circstringbuf_push(&cbuff, current_string);
        if (retval == CIRCBUF_OK) continue;
        else if (retval == CIRCBUF_DATALOSS) {
            count++;
            continue;
        }
        else TEST_ASSERT_TRUE(false);
    }
    pushindex = (pushindex + (testbuffersize - 1)) % testbuffersize;

    // pop all data which is left in the circ-buffer
    while (true) {
        int retval = circstringbuf_pop(&cbuff, &testbuffer_pop[maxstringsize * popindex++]);
        if (retval == CIRCBUF_OK)
            continue;
        else if (retval == CIRCBUF_EMPTY)
            break;
        else
            TEST_ASSERT_TRUE(false);
    }
    popindex -= 2;
    TEST_ASSERT_EQUAL(0, circstringbuf_filllevel(&cbuff));

    // compare the popped data with the last pushed
    do {
        TEST_ASSERT_EQUAL_STRING(&testbuffer[maxstringsize * pushindex],
                &testbuffer_pop[maxstringsize * popindex]);
        pushindex = (pushindex + (testbuffersize - 1)) % testbuffersize;
    } while (popindex-- > 0);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_circstringbufpush);
    RUN_TEST(test_circstringbufpop);
    RUN_TEST(test_circstringbufpopdataloss);
    RUN_TEST(test_circstringbufpushemptystring);
    RUN_TEST(test_circstringbufstuff);
    RUN_TEST(test_circstringbufrandomstrings_1);
    RUN_TEST(test_circstringbufrandomstrings_2);

    UNITY_END();
}
