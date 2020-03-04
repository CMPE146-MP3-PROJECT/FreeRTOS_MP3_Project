/* SIBROS TECHNOLOGIES, INC. CONFIDENTIAL
 * Copyright (c) 2018 - 2019 Sibros Technologies, Inc.
 * All Rights Reserved.
 * NOTICE: All information contained herein is, and remains the property of Sibros Technologies,
 * Inc. and its suppliers, if any. The intellectual and technical concepts contained herein are
 * proprietary to Sibros Technologies, Inc. and its suppliers and may be covered by U.S. and Foreign
 * Patents, patents in process, and are protected by trade secret or copyright law. Dissemination of
 * this information or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Sibros Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>

#include "unity.h"

// Module includes
#include "sl_string.c"

/******************************************************************************
 *
 *                               D E F I N E S
 *
 *****************************************************************************/
const size_t sl_header_length = sizeof(sl_string_size_t);
const char *test_very_long_string =
    "test test test test test test test test test test test test test test test test test test test test test";

/*******************************************************************************
 *
 *                                 E N U M S
 *
 ******************************************************************************/

/******************************************************************************
 *
 *                              T Y P E D E F S
 *
 *****************************************************************************/

/******************************************************************************
 *
 *        P R I V A T E    F U N C T I O N    D E C L A R A T I O N S
 *
 *****************************************************************************/
char string_memory[110];
sl_string_t str;
const size_t string_memory_pad_size = 10;
/******************************************************************************
 *
 *             P R I V A T E    D A T A    D E F I N I T I O N S
 *
 *****************************************************************************/

/******************************************************************************
 *
 *                     P R I V A T E    F U N C T I O N S
 *
 *****************************************************************************/

/******************************************************************************
 *
 *                T E S T    S E T U P    &    T E A R D O W N
 *
 *****************************************************************************/
void setUp(void) {
  memset(string_memory, 'p', sizeof(string_memory));
  str = sl_string__initialize(string_memory, sizeof(string_memory) - string_memory_pad_size);
}

void tearDown(void) {
  const char *expected_str = "pppppppppp";
  TEST_ASSERT_EQUAL_STRING(expected_str, string_memory + sizeof(string_memory) - string_memory_pad_size);
}

/******************************************************************************
 *
 *                                 T E S T S
 *
 *****************************************************************************/

void test_sl_string__initialize_successful(void) {
  TEST_ASSERT_EQUAL_PTR(string_memory + 2, str);
  TEST_ASSERT_EQUAL_UINT(sizeof(string_memory) - string_memory_pad_size, ((str[-2]) << 8) | str[-1]);
}

void test_sl_string__initialize_unsuccessful_null_mem(void) {
  sl_string_t null_str = sl_string__initialize(NULL, sizeof(string_memory) - string_memory_pad_size);

  TEST_ASSERT_EQUAL_PTR(NULL, null_str);
}

void test_sl_string__initialize_unsuccessful_mem_size_zero(void) {
  sl_string_t null_str = sl_string__initialize(string_memory, 0);

  TEST_ASSERT_EQUAL_PTR(NULL, null_str);
}

void test_sl_string__initialize_from_successful(void) {
  sl_string_t intalized_from_str =
      sl_string__initialize_from(string_memory, sizeof(string_memory) - string_memory_pad_size, "Hello World");

  TEST_ASSERT_EQUAL_PTR(string_memory + 2, str);
  TEST_ASSERT_EQUAL_UINT(sizeof(string_memory) - string_memory_pad_size, ((str[-2]) << 8) | str[-1]);
  TEST_ASSERT_EQUAL_STRING("Hello World", intalized_from_str);
}

void test_sl_string__initialize_from_null_init_string(void) {
  sl_string_t intalized_from_str =
      sl_string__initialize_from(string_memory, sizeof(string_memory) - string_memory_pad_size, NULL);

  TEST_ASSERT_EQUAL_PTR(string_memory + 2, str);
  TEST_ASSERT_EQUAL_UINT(sizeof(string_memory) - string_memory_pad_size, ((str[-2]) << 8) | str[-1]);
  TEST_ASSERT_EQUAL_STRING("", intalized_from_str);
}

void test_sl_string__initialize_from_null_string_memory(void) {
  char *str_mem = NULL;
  sl_string_t intalized_from_str = sl_string__initialize_from(str_mem, 0, NULL);

  TEST_ASSERT_EQUAL_PTR(NULL, intalized_from_str);
}

void test_sl_string__get_capacity(void) {
  TEST_ASSERT_EQUAL_UINT(sizeof(string_memory) - string_memory_pad_size - sl_header_length - 1,
                         sl_string__get_capacity(str));
}

void test_sl_string__is_full(void) {
  char mem[5];
  sl_string_t s = sl_string__initialize(mem, sizeof(mem));

  TEST_ASSERT_FALSE(sl_string__is_full(s));
  sl_string__append(s, "a");
  TEST_ASSERT_FALSE(sl_string__is_full(s));
  sl_string__append(s, "b");
  TEST_ASSERT_TRUE(sl_string__is_full(s));
}

void test_sl_string__get_length_empty_string(void) { TEST_ASSERT_EQUAL_UINT(0, sl_string__get_length(str)); }

void test_sl_string__get_length_non_empty_string(void) {
  const char *hello = "hello world";
  sl_string__set(str, hello);

  TEST_ASSERT_EQUAL_UINT(strlen(hello), sl_string__get_length(str));
}

void test_sl_string__c_str_empty_string(void) {
  TEST_ASSERT_EQUAL_STRING("", sl_string__c_str(str));
  TEST_ASSERT_EQUAL(sl_string__c_str(str), str);
}

void test_sl_string__c_str_non_empty_string(void) {
  const char *hello = "hello world";
  sl_string__set(str, hello);

  TEST_ASSERT_EQUAL_STRING(hello, sl_string__c_str(str));
  TEST_ASSERT_EQUAL(sl_string__c_str(str), str);
}

void test_append_char(void) {
  char memory[3 + 1 + sizeof(sl_string_size_t)];
  sl_string_t s = sl_string__initialize(memory, sizeof(memory));
  TEST_ASSERT_EQUAL_STRING(s, "");
  TEST_ASSERT_EQUAL(3, sl_string__get_capacity(s));
  TEST_ASSERT_EQUAL(0, sl_string__get_length(s));

  sl_string__append_char(s, 'a');
  TEST_ASSERT_EQUAL_STRING(s, "a");
  TEST_ASSERT_EQUAL(1, sl_string__get_length(s));

  sl_string__append_char(s, 'b');
  TEST_ASSERT_EQUAL_STRING(s, "ab");
  TEST_ASSERT_EQUAL(2, sl_string__get_length(s));

  sl_string__append_char(s, 'c');
  TEST_ASSERT_EQUAL_STRING(s, "abc");
  TEST_ASSERT_EQUAL(3, sl_string__get_length(s));

  sl_string__append_char(s, 'd');
  TEST_ASSERT_EQUAL_STRING(s, "abc");
  TEST_ASSERT_EQUAL(3, sl_string__get_length(s));
}

void test_sl_string__copy_to(void) {
  char copy_buffer[8];

  TEST_ASSERT_EQUAL(0, sl_string__copy_to(str, copy_buffer, 0));

  sl_string__set(str, "hello");
  TEST_ASSERT_EQUAL(0, sl_string__copy_to(str, copy_buffer, 0));

  TEST_ASSERT_EQUAL(0, sl_string__copy_to(str, copy_buffer, 1));
  TEST_ASSERT_EQUAL_STRING("", copy_buffer);

  TEST_ASSERT_EQUAL(1, sl_string__copy_to(str, copy_buffer, 2));
  TEST_ASSERT_EQUAL_STRING("h", copy_buffer);

  TEST_ASSERT_EQUAL(4, sl_string__copy_to(str, copy_buffer, 5));
  TEST_ASSERT_EQUAL_STRING("hell", copy_buffer);

  TEST_ASSERT_EQUAL(5, sl_string__copy_to(str, copy_buffer, 6));
  TEST_ASSERT_EQUAL_STRING("hello", copy_buffer);

  TEST_ASSERT_EQUAL(5, sl_string__copy_to(str, copy_buffer, 7));
  TEST_ASSERT_EQUAL_STRING("hello", copy_buffer);

  TEST_ASSERT_EQUAL(0, sl_string__copy_to(str, NULL, 0));
  TEST_ASSERT_EQUAL(0, sl_string__copy_to(str, NULL, 1));
  TEST_ASSERT_EQUAL(1, sl_string__copy_to(str, NULL, 2));
  TEST_ASSERT_EQUAL(5, sl_string__copy_to(str, NULL, 7));
}

void test_sl_string__to_int_successful(void) {
  sl_string__printf(str, "%s", "123");

  TEST_ASSERT_EQUAL_INT(123, sl_string__to_int(str));

  sl_string__printf(str, "%s", "-123");

  TEST_ASSERT_EQUAL_INT(-123, sl_string__to_int(str));
}

void test_sl_string__to_int_unsuccessful_contains_non_int_chars(void) {
  sl_string__printf(str, "%s", "Hello 123 Word");

  TEST_ASSERT_EQUAL_INT(0, sl_string__to_int(str));

  sl_string__printf(str, "%s", "$123#");

  TEST_ASSERT_EQUAL_INT(0, sl_string__to_int(str));

  sl_string__printf(str, "%s", "Hello $123# Word");

  TEST_ASSERT_EQUAL_INT(0, sl_string__to_int(str));
}

void test_sl_string__to_int_unsuccessful_empty_string(void) { TEST_ASSERT_EQUAL_INT(0, sl_string__to_int(str)); }

void test_sl_string__to_float_successful(void) {
  sl_string__printf(str, "%s", "1.23");

  TEST_ASSERT_EQUAL_FLOAT(1.23, sl_string__to_float(str));

  sl_string__printf(str, "%s", "-1.23");

  TEST_ASSERT_EQUAL_FLOAT(-1.23, sl_string__to_float(str));

  sl_string__printf(str, "%s", "+1.23f");

  TEST_ASSERT_EQUAL_FLOAT(1.23, sl_string__to_float(str));
}

void test_sl_string__to_float_unsuccessful_contains_non_float_chars(void) {
  sl_string__printf(str, "%s", "Hello 1.23 Word");

  TEST_ASSERT_EQUAL_FLOAT(0.0, sl_string__to_float(str));

  sl_string__printf(str, "%s", "$1.23#");

  TEST_ASSERT_EQUAL_FLOAT(0.0, sl_string__to_float(str));

  sl_string__printf(str, "%s", "Hello $1.23# Word");

  TEST_ASSERT_EQUAL_FLOAT(0.0, sl_string__to_float(str));
}

void test_sl_string__to_float_unsuccessful_empty_string(void) {
  TEST_ASSERT_EQUAL_FLOAT(0.0, sl_string__to_float(str));
}

void test_sl_string__is_alpha_contains_only_alpha_chars(void) {
  sl_string__set(str, "HelloWorld");
  TEST_ASSERT_TRUE(sl_string__is_alpha(str));
}

void test_sl_string__is_alpha_contains_non_alpha_chars(void) {
  sl_string__set(str, "Hello$123# Word");
  TEST_ASSERT_FALSE(sl_string__is_alpha(str));
}

void test_sl_string__is_alpha_empty_string(void) { TEST_ASSERT_TRUE(sl_string__is_alpha(str)); }

void test_sl_string__is_alphanum_successful(void) {
  sl_string__printf(str, "%s", "Hello123Word");

  TEST_ASSERT_TRUE(sl_string__is_alphanum(str));

  sl_string__printf(str, "%s", "HelloWorld");

  TEST_ASSERT_TRUE(sl_string__is_alphanum(str));

  sl_string__printf(str, "%s", "123");

  TEST_ASSERT_TRUE(sl_string__is_alphanum(str));
}

void test_sl_string__is_alphanum_contains_non_alphanum_chars(void) {
  sl_string__set(str, "Hello $123#Word");

  TEST_ASSERT_FALSE(sl_string__is_alphanum(str));
}

void test_sl_string__is_alphanum_empty_string(void) { TEST_ASSERT_TRUE(sl_string__is_alphanum(str)); }

void test_sl_string__clear(void) {
  sl_string__set(str, "Hello World");
  sl_string__clear(str);

  TEST_ASSERT_EQUAL_UINT8('\0', str[0]);
  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__clear_all_memory(void) {
  sl_string__set(str, "Hello World");

  sl_string__clear_all_memory(str);
  for (uint16_t character = 0; character < sizeof(string_memory) - string_memory_pad_size; character++) {
    TEST_ASSERT_EQUAL_UINT8('\0', str[0]);
  }

  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__to_lower(void) {
  sl_string__set(str, "Hello World");
  sl_string__to_lower(str);
  TEST_ASSERT_EQUAL_STRING("hello world", str);
}

void test_sl_string__to_upper(void) {
  sl_string__set(str, "Hello World");
  sl_string__to_upper(str);
  TEST_ASSERT_EQUAL_STRING("HELLO WORLD", str);
}

void test_sl_string__printf_successful(void) {
  const int printed_count = sl_string__printf(str, "String1 %s, Int: %d, String2: %s", "Hello", 123, "World");
  const char *expected_str = "String1 Hello, Int: 123, String2: World";

  TEST_ASSERT_EQUAL_INT(strlen(expected_str), printed_count);
  TEST_ASSERT_EQUAL_STRING(expected_str, str);
}

void test_sl_string__printf_with_string_size_zero(void) {
  sl_string_t m_str = sl_string__initialize(string_memory, sizeof(string_memory) - string_memory_pad_size);
  const int printed_count = sl_string__printf(m_str, "%s", "");
  TEST_ASSERT_EQUAL_INT(0, printed_count);
}

void test_sl_string__printf_append_with_string_size_zero(void) {
  sl_string_t m_str = sl_string__initialize(string_memory, sizeof(string_memory) - string_memory_pad_size);
  const int printed_count = sl_string__printf_append(m_str, "%s", "");
  TEST_ASSERT_EQUAL_INT(0, printed_count);
}

void test_sl_string__printf_filename_format(void) {
  const char *expected_filename = "logs/debuglog__predump.in_use";
  const sl_string_t directory_name = "logs";
  const sl_string_t preceding_name = "debuglog";
  const sl_string_t tagname = "predump";
  sl_string_t file_name = sl_string__initialize(string_memory, sizeof(string_memory) - string_memory_pad_size);
  const int printed_count = sl_string__printf(file_name, "%s/%s__%s.in_use", directory_name, preceding_name, tagname);
  TEST_ASSERT_EQUAL_INT(strlen(file_name), printed_count);
  TEST_ASSERT_EQUAL_STRING(expected_filename, file_name);
}

void test_sl_string__scanf_with_arg_size_zero(void) {
  char *hello_str = "";
  sl_string_t m_str = sl_string__initialize(string_memory, sizeof(string_memory) - string_memory_pad_size);
  int parsed_count = sl_string__scanf(m_str, "%s", hello_str);
  TEST_ASSERT_EQUAL_INT(0, parsed_count);
}

void test_sl_string__printf_truncated(void) {
  const int printed_count =
      sl_string__printf(str, "String1 %s, Int: %d, String2: %s", test_very_long_string, 123, "test");
  const char *expected_str =
      "String1 test test test test test test test test test test test test test test test test test test";
  const char *expected_str_before_truncation =
      "String1 test test test test test test test test test test test test test test test test test test test test "
      "test, Int: 123, String2: test";

  TEST_ASSERT_EQUAL_INT(strlen(expected_str_before_truncation), printed_count);
  TEST_ASSERT_EQUAL_STRING(expected_str, str);
}

void test_sl_string__printf_append_successful(void) {
  sl_string__append(str, "Append ");
  const int printed_count = sl_string__printf_append(str, "String1 %s, Int: %d, String2: %s", "Hello", 123, "World");
  const char *expected_str_appended = "String1 Hello, Int: 123, String2: World";
  const char *expected_str = "Append String1 Hello, Int: 123, String2: World";

  TEST_ASSERT_EQUAL_INT(strlen(expected_str_appended), printed_count);
  TEST_ASSERT_EQUAL_STRING(expected_str, str);
}

void test_sl_string__printf_append_truncated(void) {
  sl_string__append(str, "Append ");
  int printed_count =
      sl_string__printf_append(str, "String1 %s, Int: %d, String2: %s", test_very_long_string, 123, "test");
  const char *expected_str =
      "Append String1 test test test test test test test test test test test test test test test test te";
  const char *expected_str_appended_before_truncation =
      "String1 test test test test test test test test test test test test test test test test test test test test "
      "test, Int: 123, String2: test";

  TEST_ASSERT_EQUAL_INT(strlen(expected_str_appended_before_truncation), printed_count);
  TEST_ASSERT_EQUAL_STRING(expected_str, str);

  printed_count = sl_string__printf_append(str, "%s", test_very_long_string);

  TEST_ASSERT_EQUAL_INT(strlen(test_very_long_string), printed_count);
  TEST_ASSERT_EQUAL_STRING(expected_str, str);
}

void test_sl_string__scanf_successful(void) {
  char string1[20];
  char string2[20];
  int num = 0;

  sl_string__set(str, "Hello 123 World");
  const int scanned_count = sl_string__scanf(str, "%20s %d %20s", string1, &num, string2);

  TEST_ASSERT_EQUAL_INT(3, scanned_count);
  TEST_ASSERT_EQUAL_STRING("Hello", string1);
  TEST_ASSERT_EQUAL_INT(123, num);
  TEST_ASSERT_EQUAL_STRING("World", string2);
}

void test_sl_string__tokenize_successful(void) {
  sl_string__set(str, "Hello 123 World");

  char *one = NULL;
  char *two = NULL;
  char *three = NULL;

  TEST_ASSERT_EQUAL_INT(3, sl_string__tokenize(str, " ", 3, &one, &two, &three));

  TEST_ASSERT_EQUAL_STRING("Hello", one);
  TEST_ASSERT_EQUAL_STRING("123", two);
  TEST_ASSERT_EQUAL_STRING("World", three);
}

void test_sl_string__tokenize_unsuccessful_null_delimiter(void) {
  sl_string__set(str, "Hello 123 Word");

  char *one = NULL;
  char *two = NULL;
  char *three = NULL;

  TEST_ASSERT_EQUAL_INT(-1, sl_string__tokenize(str, NULL, 3, &one, &two, &three));

  TEST_ASSERT_EQUAL_STRING(NULL, one);
  TEST_ASSERT_EQUAL_STRING(NULL, two);
  TEST_ASSERT_EQUAL_STRING(NULL, three);
}

void test_sl_string__tokenize_null_args(void) {
  sl_string__set(str, "Hello,123,Word");
  TEST_ASSERT_EQUAL_INT(1, sl_string__tokenize(str, ",", 1, NULL));
}

void test_sl_string__tokenize_mismatch_of_ptrs_and_num_of_tokens(void) {
  sl_string__set(str, "Hello 123 Word");

  char *one = NULL;
  char *two = NULL;
  char *three = NULL;

  TEST_ASSERT_EQUAL_INT(2, sl_string__tokenize(str, " ", 2, &one, &two));

  TEST_ASSERT_EQUAL_STRING("Hello", one);
  TEST_ASSERT_EQUAL_STRING("123", two);
  TEST_ASSERT_EQUAL_STRING(NULL, three);
}

void test_sl_string__set(void) {
  TEST_ASSERT_TRUE(sl_string__set(str, "hello"));
  TEST_ASSERT_EQUAL_STRING("hello", str);

  // When string does not have enough memory (capacity is 2 when memory length is 5)
  str = sl_string__initialize(string_memory, 5);
  TEST_ASSERT_TRUE(sl_string__set(str, "hi"));
  TEST_ASSERT_EQUAL_STRING("hi", str);
  TEST_ASSERT_TRUE(sl_string__set(str, "I"));
  TEST_ASSERT_EQUAL_STRING("I", str);

  TEST_ASSERT_FALSE(sl_string__set(str, "aaaaaaaaaaaaaaaaa"));
  TEST_ASSERT_EQUAL_STRING("I", str);
}

void test_sl_string__insert_at_success(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__insert_at(str, 5, "Hello 123 World"));
  TEST_ASSERT_EQUAL_STRING("HelloHello 123 World World", str);
}

void test_sl_string__insert_at_pos_out_of_bounds(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__insert_at(str, sizeof("Hello World"), "Hello 123 Word"));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__insert_at_string_is_null(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__insert_at(str, 5, NULL));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__insert_at_capacity_reached(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__insert_at(str, 5, test_very_long_string));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__append_successful(void) {
  TEST_ASSERT_TRUE(sl_string__append(str, "Hello World"));

  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__append_string_is_null(void) {
  TEST_ASSERT_FALSE(sl_string__append(str, NULL));

  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__append_capacity_reached(void) {
  TEST_ASSERT_FALSE(sl_string__append(str, test_very_long_string));

  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__equals_to_successful(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__equals_to(str, "Hello World"));
}

void test_sl_string__equals_to_unsuccessful_strings_dont_match(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__equals_to(str, "hello world"));
}

void test_sl_string__equals_to_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__equals_to(str, NULL));
}

void test_sl_string__equals_to_ignore_case_successful(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__equals_to_ignore_case(str, "HelLo WoRld"));
  TEST_ASSERT_TRUE(sl_string__equals_to_ignore_case(str, "hello world"));
}

void test_sl_string__equals_to_ignore_case_unsuccessful_strings_dont_match(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__equals_to_ignore_case(str, "Hello 123 Word"));
}

void test_sl_string__equals_to_ignore_case_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__equals_to_ignore_case(str, NULL));
}

void test_sl_string__last_index_of_found(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(12, sl_string__last_index_of(str, "Hello"));
}

void test_sl_string__last_index_of_unsuccessful_not_found(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__last_index_of(str, "hello"));
}

void test_sl_string__last_index_of_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__last_index_of(str, NULL));
}

void test_sl_string__last_index_of_ignore_case_found(void) {
  sl_string__printf(str, "Hello World Hello World");

  TEST_ASSERT_EQUAL_INT(12, sl_string__last_index_of_ignore_case(str, "hello"));

  sl_string__printf(str, "Hello World HHello World");

  TEST_ASSERT_EQUAL_INT(13, sl_string__last_index_of_ignore_case(str, "hello"));
}

void test_sl_string_last_index_of_ignore_lower_case(void) {
  sl_string__set(str, "hello WORLD");
  TEST_ASSERT_EQUAL_INT(0, sl_string__last_index_of_ignore_case(str, "HELLO"));
}

void test_sl_string_last_index_of_ignore_multiple_sub_string(void) {
  sl_string__set(str, "hello WORLD hello hello");
  TEST_ASSERT_EQUAL_INT(18, sl_string__last_index_of_ignore_case(str, "hello"));
}

void test_sl_string__last_index_of_ignore_case_not_found(void) {
  TEST_ASSERT_EQUAL_INT(-1, sl_string__last_index_of_ignore_case(str, "hey"));
}

void test_sl_string__last_index_of_ignore_case_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__last_index_of_ignore_case(str, NULL));
}

void test_sl_string__first_index_of_found(void) {
  sl_string__printf(str, "%s", "Hello World Hello World");

  TEST_ASSERT_EQUAL_INT(0, sl_string__first_index_of(str, "Hello"));

  sl_string__printf(str, "%s", "HHello World Hello World");

  TEST_ASSERT_EQUAL_INT(1, sl_string__first_index_of(str, "Hello"));
}

void test_sl_string__first_index_of_unsuccessful_not_found(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__first_index_of(str, "hello"));
}

void test_sl_string__first_index_of_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__first_index_of(str, NULL));
}

void test_sl_string__first_index_of_ignore_case_found(void) {
  sl_string__printf(str, "%s", "Hello World Hello World");

  TEST_ASSERT_EQUAL_INT(0, sl_string__first_index_of_ignore_case(str, "hello"));

  sl_string__printf(str, "%s", "HHello World Hello World");

  TEST_ASSERT_EQUAL_INT(1, sl_string__first_index_of(str, "Hello"));
}

void test_sl_string__first_index_of_ignore_case_not_found(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__first_index_of_ignore_case(str, "hey"));
}

void test_sl_string__first_index_of_ignore_case_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__first_index_of_ignore_case(str, NULL));
}

void test_sl_string_first_index_of_ignore_lower_case(void) {
  sl_string__set(str, "hello HeLLO WORLD");
  TEST_ASSERT_EQUAL_INT(0, sl_string__first_index_of_ignore_case(str, "HeLLO"));
}

void test_sl_string__contains_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__contains(str, "Hello"));
}

void test_sl_string__contains_unsuccessful_not_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__contains(str, "hello"));
}

void test_sl_string__contains_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__contains(str, NULL));
}

void test_sl_string__contains_ignore_case_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__contains_ignore_case(str, "hello"));
}

void test_sl_string__contains_ignore_case_not_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__contains_ignore_case(str, "hey"));
}

void test_sl_string__contains_ignore_case_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__contains_ignore_case(str, NULL));
}

void test_sl_string__count_of_successful(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(2, sl_string__count_of(str, "World"));
}

void test_sl_string__count_of_no_occurance(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(0, sl_string__count_of(str, "hey"));
}

void test_sl_string__count_of_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World Hello World");
  TEST_ASSERT_EQUAL_INT(-1, sl_string__count_of(str, NULL));
}

void test_sl_string__begins_with_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__begins_with(str, "Hello"));
}

void test_sl_string__begins_with_unsuccessful_not_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__begins_with(str, "World"));
}

void test_sl_string__begins_with_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__begins_with(str, NULL));
}

void test_sl_string__begins_unsuccessful_large_substring(void) {
  sl_string__set(str, "world");
  TEST_ASSERT_FALSE(sl_string__begins_with(str, "Hello World"));
}

void test_sl_string__begins_with_limited_chars(void) {
  sl_string__set(str, "test begins with");

  TEST_ASSERT_TRUE(sl_string__begins_with_limited_chars(str, "test", 1));
  TEST_ASSERT_TRUE(sl_string__begins_with_limited_chars(str, "test", 2));
  TEST_ASSERT_TRUE(sl_string__begins_with_limited_chars(str, "test", 3));
  TEST_ASSERT_TRUE(sl_string__begins_with_limited_chars(str, "test", 4));

  TEST_ASSERT_TRUE(sl_string__begins_with_limited_chars(str, "test ", 5));
  TEST_ASSERT_TRUE(sl_string__begins_with_limited_chars(str, "test b", 6));

  TEST_ASSERT_FALSE(sl_string__begins_with_limited_chars(str, "tesT", 4));
  TEST_ASSERT_FALSE(sl_string__begins_with_limited_chars(str, "ab", 2));
  TEST_ASSERT_FALSE(sl_string__begins_with_limited_chars(str, "tx", 2));
}

void test_sl_string__begins_with_ignore_case_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__begins_with_ignore_case(str, "hElLo"));
}

void test_sl_string__begins_with_ignore_case_not_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__begins_with_ignore_case(str, "hey"));
}

void test_sl_string__begins_with_ignore_case_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__begins_with_ignore_case(str, NULL));
}

void test_sl_string__begins_ignore_case_unsuccessful_large_substring(void) {
  sl_string__set(str, "world");
  TEST_ASSERT_FALSE(sl_string__begins_with_ignore_case(str, "hEllo World"));
}

void test_sl_string__begins_with_whole_word_found(void) {
  sl_string__printf(str, "%s", "Hello World");

  TEST_ASSERT_TRUE(sl_string__begins_with_whole_word(str, "Hello", " "));

  sl_string__printf(str, "%s", "Hello;World");

  TEST_ASSERT_TRUE(sl_string__begins_with_whole_word(str, "Hello", ";"));
}

void test_sl_string__begins_with_whole_word_unsuccessful_large_substring(void) {
  sl_string__set(str, "World");
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word(str, "Hello World", ""));
}

void test_sl_string__begins_with_whole_word_not_matching(void) {
  sl_string__set(str, "World");
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word(str, "orld", ""));
}

void test_sl_string__begins_with_whole_word_string_ends_with_null(void) {
  sl_string__set(str, "World\0");
  TEST_ASSERT_TRUE(sl_string__begins_with_whole_word(str, "World", ","));
}

void test_sl_string__begins_with_whole_word_unsuccessful_not_found(void) {
  sl_string__set(str, "HelloWorld");
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word(str, "Hello", " "));
}

void test_sl_string__begins_with_whole_word_unsuccessful_null_params(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word(str, NULL, " "));
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word(str, "Hello", NULL));
}

void test_sl_string__begins_with_whole_word_ignore_case_found(void) {
  sl_string__printf(str, "%s", "Hello World");

  TEST_ASSERT_TRUE(sl_string__begins_with_whole_word_ignore_case(str, "HeLlO", " "));

  sl_string__printf(str, "%s", "Hello;World");

  TEST_ASSERT_TRUE(sl_string__begins_with_whole_word_ignore_case(str, "heLlO", ";"));
}

void test_sl_string__begins_with_whole_word_ignore_case_not_found(void) {
  sl_string__set(str, "HelloWorld");
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word_ignore_case(str, "HeLlO", " "));
}

void test_sl_string__begins_with_whole_word_ignore_case_unsuccessful_null_params(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word_ignore_case(str, NULL, " "));
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word_ignore_case(str, "Hello", NULL));
}

void test_sl_string__begins_with_whole_word_ignore_case_unsuccessful_large_substring(void) {
  sl_string__set(str, "World");
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word_ignore_case(str, "Hello World", ""));
}

void test_sl_string__begins_with_whole_word_ignore_case_not_matching(void) {
  sl_string__set(str, "World");
  TEST_ASSERT_FALSE(sl_string__begins_with_whole_word_ignore_case(str, "orld", ""));
}

void test_sl_string__begins_with_whole_word_ignore_case_string_delimeter_null(void) {
  sl_string__set(str, "World,");
  TEST_ASSERT_TRUE(sl_string__begins_with_whole_word_ignore_case(str, "World", "\0"));
}

void test_sl_string__ends_with_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_TRUE(sl_string__ends_with(str, "World"));
}

void test_sl_string__ends_with_unsuccessful_large_substring(void) {
  sl_string__set(str, "Hello");
  TEST_ASSERT_FALSE(sl_string__ends_with(str, "Hello World"));
}

void test_sl_string__ends_with_unsuccessful_not_found(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__ends_with(str, "Hello"));
}

void test_sl_string__ends_with_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__ends_with(str, NULL));
}

void test_sl_string__ends_newline(void) {
  sl_string__set(str, "Hello World\r");
  TEST_ASSERT_TRUE(sl_string__ends_with_newline(str));
  sl_string__set(str, "Hello World\r\n");
  TEST_ASSERT_TRUE(sl_string__ends_with_newline(str));
  sl_string__set(str, "Hello World\n");
  TEST_ASSERT_TRUE(sl_string__ends_with_newline(str));
  sl_string__set(str, "Hello World");
  TEST_ASSERT_FALSE(sl_string__ends_with_newline(str));

  sl_string__set(str, "");
  TEST_ASSERT_FALSE(sl_string__ends_with_newline(str));
}

void test_sl_string__ends_with_ignore_case_found(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__ends_with_ignore_case(str, "wOrLd"));
}

void test_sl_string__ends_with_ignore_case_unsuccessful_large_substring(void) {
  sl_string__set(str, "Hello");

  TEST_ASSERT_FALSE(sl_string__ends_with_ignore_case(str, "Hello World"));
}

void test_sl_string__ends_with_ignore_case_not_found(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__ends_with_ignore_case(str, "hELLo"));
}

void test_sl_string__ends_with_ignore_case_unsuccessful_null_substring(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__ends_with_ignore_case(str, NULL));
}

void test_sl_string__erase_successful(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase(str, "Hello"));
  TEST_ASSERT_EQUAL_STRING(" World", str);
}

void test_sl_string__erase_null_substring(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__erase(str, NULL));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__erase_substring_not_found(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__erase(str, "hey"));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__erase_first_successful(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_first(str, 4));
  TEST_ASSERT_EQUAL_STRING("o World", str);
}

void test_sl_string__erase_first_n_chars_longer_than_string(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_first(str, sl_string__get_length(str) + 1));
  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__erase_last_successful(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_last(str, 4));
  TEST_ASSERT_EQUAL_STRING("Hello W", str);
}

void test_sl_string__erase_last_n_chars_longer_than_string(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_last(str, sl_string__get_length(str) + 1));
  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__erase_at_successful(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_at(str, 6));
  TEST_ASSERT_EQUAL_STRING("Hello orld", str);
}

void test_sl_string__erase_at_index_out_of_bounds(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__erase_at(str, sl_string__get_length(str) + 1));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__erase_after_successful(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_after(str, 2, 3));
  TEST_ASSERT_EQUAL_STRING("He World", str);
}

void test_sl_string__erase_after_index_out_of_bounds(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_FALSE(sl_string__erase_after(str, sl_string__get_length(str) + 1, 3));
  TEST_ASSERT_EQUAL_STRING("Hello World", str);
}

void test_sl_string__erase_after_n_chars_longer_than_string(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_after(str, 0, sl_string__get_length(str) + 1));
  TEST_ASSERT_EQUAL_STRING("", str);
}

void test_sl_string__erase_after_string(void) {
  sl_string__set(str, "Hello World 123");

  TEST_ASSERT_TRUE(sl_string__erase_at_substring(str, " World"));
  TEST_ASSERT_EQUAL_STRING("Hello", str);

  TEST_ASSERT_FALSE(sl_string__erase_at_substring(str, "none"));
  TEST_ASSERT_EQUAL_STRING("Hello", str);
}

void test_sl_string__erase_first_word_successful(void) {
  sl_string__set(str, "Hello World");

  TEST_ASSERT_TRUE(sl_string__erase_first_word(str, ' '));
  TEST_ASSERT_EQUAL_STRING("World", str);
}

void test_sl_string__erase_first_word_no_words(void) {
  sl_string__set(str, "HelloWorld");

  TEST_ASSERT_FALSE(sl_string__erase_first_word(str, ' '));
  TEST_ASSERT_EQUAL_STRING("HelloWorld", str);
}

void test_sl_string__erase_special_chars_successful(void) {
  sl_string__set(str, "He@llo $123& Wor*ld");

  TEST_ASSERT_EQUAL_UINT16(6, sl_string__erase_special_chars(str));
  TEST_ASSERT_EQUAL_STRING("Hello123World", str);
}

void test_sl_string__erase_special_chars_no_special_chars(void) {
  sl_string__set(str, "Hello123World");

  TEST_ASSERT_EQUAL_UINT16(0, sl_string__erase_special_chars(str));
  TEST_ASSERT_EQUAL_STRING("Hello123World", str);
}

void test_erase_int(void) {
  sl_string_t s =
      sl_string__initialize_from(string_memory, sizeof(string_memory) - string_memory_pad_size, "111 222ab333 ;a.444");
  int result = -1;

  TEST_ASSERT_TRUE(sl_string__erase_int(s, &result));
  TEST_ASSERT_EQUAL_INT(111, result);

  TEST_ASSERT_TRUE(sl_string__erase_int(s, &result));
  TEST_ASSERT_EQUAL_INT(222, result);

  TEST_ASSERT_TRUE(sl_string__erase_int(s, &result));
  TEST_ASSERT_EQUAL_INT(333, result);

  TEST_ASSERT_TRUE(sl_string__erase_int(s, &result));
  TEST_ASSERT_EQUAL_INT(444, result);

  TEST_ASSERT_FALSE(sl_string__erase_int(s, &result));
  TEST_ASSERT_EQUAL_INT(444, result);
  TEST_ASSERT_FALSE(sl_string__erase_int(s, &result));
  TEST_ASSERT_EQUAL_INT(444, result);

  sl_string__set(s, "123");
  TEST_ASSERT_FALSE(sl_string__erase_int(s, NULL));
  TEST_ASSERT_EQUAL_STRING(s, "");
}

void test_sl_string__trim_end_successful(void) {
  sl_string__set(str, "./././123 ***");

  TEST_ASSERT_TRUE(sl_string__trim_end(str, "./*"));
  TEST_ASSERT_EQUAL_STRING("./././123 ", str);
}

void test_sl_string__trim_end_null_chars_to_trim(void) {
  sl_string__set(str, "./././123 ***");

  TEST_ASSERT_FALSE(sl_string__trim_end(str, NULL));
  TEST_ASSERT_EQUAL_STRING("./././123 ***", str);
}

void test_sl_string__trim_end_chars_not_found(void) {
  sl_string__set(str, "./././123 ***");

  TEST_ASSERT_TRUE(sl_string__trim_end(str, "./"));
  TEST_ASSERT_EQUAL_STRING("./././123 ***", str);
}

void test_sl_string__trim_start_successful(void) {
  sl_string__set(str, "./././123 ***");

  TEST_ASSERT_TRUE(sl_string__trim_start(str, "./*"));
  TEST_ASSERT_EQUAL_STRING("123 ***", str);

  TEST_ASSERT_TRUE(sl_string__trim_start(str, "./*"));
  TEST_ASSERT_EQUAL_STRING("123 ***", str);
}

void test_sl_string__trim_start_null_chars_to_trim(void) {
  sl_string__set(str, "./././123 ***");

  TEST_ASSERT_FALSE(sl_string__trim_start(str, NULL));
  TEST_ASSERT_EQUAL_STRING("./././123 ***", str);
}

void test_sl_string__trim_start_chars_not_found(void) {
  sl_string__set(str, "./././123 ***");

  TEST_ASSERT_TRUE(sl_string__trim_start(str, "*"));
  TEST_ASSERT_EQUAL_STRING("./././123 ***", str);
}

void test_sl_string__replace_first_successful(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_TRUE(sl_string__replace_first(str, "Hello", "World"));
  TEST_ASSERT_EQUAL_STRING("World World Hello", str);
}

void test_sl_string__replace_first_null_params(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_FALSE(sl_string__replace_first(str, NULL, "World"));
  TEST_ASSERT_FALSE(sl_string__replace_first(str, "Hello", NULL));
  TEST_ASSERT_FALSE(sl_string__replace_first(str, NULL, NULL));

  TEST_ASSERT_EQUAL_STRING("Hello World Hello", str);
}

void test_sl_string__replace_first_substring_not_found(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_TRUE(sl_string__replace_first(str, "hey", "World"));
  TEST_ASSERT_EQUAL_STRING("Hello World Hello", str);
}

void test_sl_string__replace_first_empty_string_and_replace_string(void) {
  sl_string__set(str, "");
  TEST_ASSERT_TRUE(sl_string__replace_first(str, "", "World"));
  TEST_ASSERT_EQUAL_STRING("World", str);
}

void test_sl_string__replace_last_successful(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_TRUE(sl_string__replace_last(str, "Hello", "World"));
  TEST_ASSERT_EQUAL_STRING("Hello World World", str);
}

void test_sl_string__replace_last_null_params(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_FALSE(sl_string__replace_last(str, NULL, "World"));
  TEST_ASSERT_FALSE(sl_string__replace_last(str, "Hello", NULL));
  TEST_ASSERT_FALSE(sl_string__replace_last(str, NULL, NULL));

  TEST_ASSERT_EQUAL_STRING("Hello World Hello", str);
}

void test_sl_string__replace_last_substring_not_found(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_TRUE(sl_string__replace_last(str, "hey", "World"));
  TEST_ASSERT_EQUAL_STRING("Hello World Hello", str);
}

void test_sl_string__replace_all_successful(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_EQUAL_INT(2, sl_string__replace_all(str, "Hello", "World"));
  TEST_ASSERT_EQUAL_STRING("World World World", str);
}

void test_sl_string__replace_all_null_params(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_EQUAL_INT(-1, sl_string__replace_all(str, NULL, "World"));
  TEST_ASSERT_EQUAL_INT(-1, sl_string__replace_all(str, "Hello", NULL));
  TEST_ASSERT_EQUAL_INT(-1, sl_string__replace_all(str, NULL, NULL));

  TEST_ASSERT_EQUAL_STRING("Hello World Hello", str);
}

void test_sl_string__replace_all_substring_not_found(void) {
  sl_string__set(str, "Hello World Hello");

  TEST_ASSERT_EQUAL_INT(0, sl_string__replace_all(str, "hey", "World"));
  TEST_ASSERT_EQUAL_STRING("Hello World Hello", str);
}

void test_sl_string__replace_all_double_slash_with_slash(void) {
  const char *expected_filename = "sibros/can_module/logs/debuglog__predump.in_use";
  const sl_string_t directory_name = "sibros/can_module/logs/";
  const sl_string_t preceding_name = "debuglog";
  const sl_string_t tagname = "predump";
  sl_string_t file_name = sl_string__initialize(string_memory, sizeof(string_memory) - string_memory_pad_size);
  const int printed_count = sl_string__printf(file_name, "%s/%s__%s.in_use", directory_name, preceding_name, tagname);
  TEST_ASSERT_EQUAL_INT(strlen(file_name), printed_count);
  // replace double slash with slash from middle of the string
  TEST_ASSERT_EQUAL_INT(1, sl_string__replace_all(file_name, "//", "/"));
  TEST_ASSERT_EQUAL_STRING(expected_filename, file_name);

  sl_string_t string_with_slash = "/sibros/can_module/logs/debuglog__predump.in_use";
  // adding slash in the beginning of string_with_slash
  sl_string__printf(file_name, "/%s", string_with_slash); // re-use file_name
  // replace double slash with slash from beginning of the string
  TEST_ASSERT_EQUAL_INT(1, sl_string__replace_all(file_name, "//", "/"));
  TEST_ASSERT_EQUAL_STRING(string_with_slash, file_name);

  string_with_slash = "/sibros/can_module/logs/";
  // adding slash at the end of string_with_slash
  sl_string__printf(file_name, "%s/", string_with_slash); // re-use file_name
  // replace double slash with slash from end of the string
  TEST_ASSERT_EQUAL_INT(1, sl_string__replace_all(file_name, "//", "/"));
  TEST_ASSERT_EQUAL_STRING(string_with_slash, file_name);
}
