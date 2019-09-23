#include <stdio.h>
#include <stdint.h>
#include "unity.h"

#include "../circstringbuf.h"

#define BUFFER_SIZE (20)

static char buffer[BUFFER_SIZE];

static circstringbuf_t cbuff;

void printb()
{
    int ii;
    for (ii = 0; ii < BUFFER_SIZE; ii++)
    {
        if (buffer[ii])
            printf("%c", buffer[ii]);
        else
            printf("-");
    }

    printf("\n");
    for (ii = 0; ii < cbuff.current_start; ii++) printf(" ");
    printf("*\n");
    for (ii = 0; ii < cbuff.current_end; ii++) printf(" ");
    printf("#\n");
}

void setUp(void)
{
    circstringbuf_init(&cbuff, buffer, sizeof(buffer));
}

void tearDown(void)
{
}

void test_circstringbufpush(void)
{
    TEST_ASSERT_EQUAL(CIRCBUF_ERROR, circstringbuf_push(&cbuff, "12345678901234567890"));

    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test1"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test2"));
    TEST_ASSERT_EQUAL(CIRCBUF_OK, circstringbuf_push(&cbuff, "test3"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test4"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test5"));
    TEST_ASSERT_EQUAL(CIRCBUF_DATALOSS, circstringbuf_push(&cbuff, "test6"));
}

void test_circstringbufpop(void)
{
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

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_circstringbufpush);
    RUN_TEST(test_circstringbufpop);
    RUN_TEST(test_circstringbufpopdataloss);
    RUN_TEST(test_circstringbufpushemptystring);
    UNITY_END();
}
