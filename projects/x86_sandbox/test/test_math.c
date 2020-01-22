#include "unity.h"

#include "math.c"

void setUp(void) {

}

void tearDown(void) {

}

void test_add(void) {
    TEST_ASSERT_EQUAL(3, math__add(2, 1));
    TEST_ASSERT_EQUAL(3, math__vars.last_result);
    TEST_ASSERT_EQUAL(3, math__private_get_last_result());
}
