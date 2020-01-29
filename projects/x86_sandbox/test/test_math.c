#include "Mockdummy_module.h"

#include "unity.h"

#include "math.c"

void setUp(void) {}

void tearDown(void) {}

void test_add(void) {
  TEST_ASSERT_EQUAL(3, math__add(2, 1));
  TEST_ASSERT_EQUAL(3, math__vars.last_result);
  TEST_ASSERT_EQUAL(3, math__private_get_last_result());
}

void test_dummy_module(void) {
  dummy_module__get_zero_ExpectAndReturn(0);
  TEST_ASSERT_EQUAL(0, math__return_zero());

  dummy_module__get_zero_ExpectAndReturn(1);
  TEST_ASSERT_EQUAL(1, math__return_zero());
}
