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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "sl_string.h"

/*******************************************************************************
 *
 *               P R I V A T E    D A T A    D E F I N I T I O N S
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *                     P R I V A T E    F U N C T I O N S
 *
 ******************************************************************************/

void sl_byte_convert__to_network_from_uint16(uint8_t bytes[2], uint16_t source_value) {
  bytes[0] = (uint8_t)(source_value >> 8);
  bytes[1] = (uint8_t)(source_value >> 0);
}

uint16_t sl_byte_convert__from_network_to_uint16(const uint8_t bytes[2]) {
  return (((uint16_t)bytes[0] << 8) | (uint16_t)bytes[1]);
}

int sl_utils__pointer_distance(const void *pointer_to_last, const void *string) { return (pointer_to_last - string); }

static sl_string_size_t sl_string__private_return_positive_or_zero(const int value) {
  return (value > 0) ? (sl_string_size_t)value : 0U;
}

static bool sl_string__private_ensure_enough_memory_for_insertion(const sl_string_t string,
                                                                  const char *insertion_string) {
  const sl_string_size_t length_of_original_string = sl_string__get_length(string);
  const sl_string_size_t length_of_insertion_string = (sl_string_size_t)strlen(insertion_string);
  const sl_string_size_t memory_required = length_of_original_string + length_of_insertion_string;

  return memory_required <= sl_string__get_capacity(string);
}

/**
 * Some compilers do not offer strtok_r() hence we had to port this over
 */
static char *sl_string__private_strtok_r(sl_string_t string, const char *delimiter_string, char **save_ptr) {
  char *end_of_token = NULL;
  char *token = NULL;

  if (NULL == string) {
    string = *save_ptr;
  }

  if (0U != sl_string__get_length(string)) {
    string += strspn(string, delimiter_string);
    *save_ptr = string;

    if (0U != sl_string__get_length(string)) {
      end_of_token = string + strcspn(string, delimiter_string);
      *save_ptr = end_of_token;
      token = string;

      if ('\0' != *end_of_token) {
        *end_of_token = '\0';
        *save_ptr = end_of_token + 1;
        token = string;
      }
    }
  }

  return token;
}

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

sl_string_t sl_string__initialize(void *memory, sl_string_size_t memory_size_max) {
  sl_string_t string = NULL;

  // We need memory to hold sizeof(sl_string_size_t) plus 1 char for a meaningful string and 1 char for NULL
  if ((NULL != memory) && (memory_size_max >= (sizeof(sl_string_size_t) + 2U))) {
    memset(memory, 0, memory_size_max);

    void *header = memory;
    void *string_start = (memory + sizeof(sl_string_size_t));

    sl_byte_convert__to_network_from_uint16(header, memory_size_max);

    string = (sl_string_t)string_start;
  }

  return string;
}

sl_string_t sl_string__initialize_from(void *memory, sl_string_size_t memory_size_max, const char *copy_from) {
  sl_string_t string = sl_string__initialize(memory, memory_size_max);

  if (NULL != string) {
    (void)sl_string__set(string, copy_from);
  }

  return string;
}

sl_string_size_t sl_string__get_capacity(const sl_string_t string) {
  const sl_string_size_t capacity =
      sl_byte_convert__from_network_to_uint16((const uint8_t *)(string - sizeof(sl_string_size_t)));

  // It is okay to to subtract (sizeof() + 1U) from capacity as we
  // check to ensure the memory meets a minimum value during initialization.
  return (sl_string_size_t)(capacity - sizeof(sl_string_size_t) - 1U);
}

sl_string_size_t sl_string__get_capacity_including_null_char(const sl_string_t string) {
  return sl_string__get_capacity(string) + 1U;
}

sl_string_size_t sl_string__get_length(const sl_string_t string) {
  return (NULL == string) ? 0U : (sl_string_size_t)strlen(string);
}

bool sl_string__is_full(const sl_string_t string) {
  const sl_string_size_t length = sl_string__get_length(string);
  const sl_string_size_t capacity = sl_string__get_capacity(string);
  return (length >= capacity);
}

const char *sl_string__c_str(const sl_string_t string) { return string; }

int sl_string__to_int(const sl_string_t string) { return atoi(string); }

double sl_string__to_float(const sl_string_t string) { return atof(string); }

bool sl_string__is_alpha(const sl_string_t string) {
  bool is_alpha = true;
  const sl_string_size_t string_length = sl_string__get_length(string);

  for (sl_string_size_t character = 0; character < string_length; character++) {
    if (0U == isalpha((int)string[character])) {
      is_alpha = false;
      break;
    }
  }

  return is_alpha;
}

bool sl_string__is_alphanum(const sl_string_t string) {
  bool is_alpha_num = true;
  const sl_string_size_t string_length = sl_string__get_length(string);

  for (sl_string_size_t character = 0; character < string_length; character++) {
    if (0U == isalnum((int)string[character])) {
      is_alpha_num = false;
      break;
    }
  }

  return is_alpha_num;
}

void sl_string__clear(sl_string_t string) { *string = '\0'; }

void sl_string__clear_all_memory(sl_string_t string) { memset(string, 0, sl_string__get_capacity(string)); }

void sl_string__to_lower(sl_string_t string) {
  sl_string_t iteration_string = string;
  while ('\0' != *iteration_string) {
    *iteration_string = (char)tolower((int)(*iteration_string));
    iteration_string++;
  }
}

void sl_string__to_upper(sl_string_t string) {
  sl_string_t iterator = string;
  while ('\0' != *iterator) {
    *iterator = (char)toupper((int)(*iterator));
    iterator++;
  }
}

sl_string_size_t sl_string__printf(sl_string_t string, const char *format, ...) {
  va_list args;
  va_start(args, format);
  const sl_string_size_t chars_printed = sl_string__vprintf(string, format, args);
  va_end(args);

  return chars_printed;
}

sl_string_size_t sl_string__vprintf(sl_string_t string, const char *format, va_list args) {
  const sl_string_size_t capacity = sl_string__get_capacity_including_null_char(string);
  const int chars_printed = vsnprintf(string, capacity, format, args);
  return sl_string__private_return_positive_or_zero(chars_printed);
}

sl_string_size_t sl_string__printf_append(sl_string_t string, const char *format, ...) {
  va_list args;
  va_start(args, format);
  const sl_string_size_t chars_printed = sl_string__vprintf_append(string, format, args);
  va_end(args);

  return chars_printed;
}

sl_string_size_t sl_string__vprintf_append(sl_string_t string, const char *format, va_list args) {
  const sl_string_size_t existing_length = sl_string__get_length(string);
  const sl_string_size_t remaining_capacity = sl_string__get_capacity_including_null_char(string) - existing_length;
  const int chars_printed = vsnprintf(string + existing_length, remaining_capacity, format, args);

  return sl_string__private_return_positive_or_zero(chars_printed);
}

size_t sl_string__scanf(const sl_string_t string, const char *format, ...) {
  va_list args;
  va_start(args, format);

  const int number_of_parsed_params = vsscanf(string, format, args);
  va_end(args);

  return (number_of_parsed_params > 0) ? (sl_string_size_t)number_of_parsed_params : 0U;
}

int sl_string__tokenize(sl_string_t string, const char *delimiter_string, int char_ptr_count, ...) {
  int token_count = sl_string_error;

  if (NULL != delimiter_string) {
    va_list args;
    va_start(args, char_ptr_count);

    token_count = 0;
    char *save_pointer = NULL;
    char *token = sl_string__private_strtok_r(string, delimiter_string, &save_pointer);

    while ((NULL != token) && (char_ptr_count > 0)) {
      char **param_argument = va_arg(args, char **);
      if (NULL != param_argument) {
        *param_argument = token;
      }

      token = sl_string__private_strtok_r(NULL, delimiter_string, &save_pointer);

      char_ptr_count--;
      token_count++;
    }
    va_end(args);
  }

  return token_count;
}

bool sl_string__set(sl_string_t string, const char *source) {
  bool result = false;

  if (NULL != source) {
    const sl_string_size_t source_length = (sl_string_size_t)strlen(source);
    const sl_string_size_t destination_length = sl_string__get_capacity(string);
    if (destination_length >= source_length) {
      strncpy(string, source, destination_length);
      result = true;
    }
  }

  return result;
}

bool sl_string__insert_at(sl_string_t string, sl_string_size_t index_position, const char *insert) {
  bool result = false;

  if (NULL != insert) {
    if (index_position <= sl_string__get_length(string)) {
      if (sl_string__private_ensure_enough_memory_for_insertion(string, insert)) {
        const size_t length_of_insertion_string = strlen(insert);

        char *position_of_insertion = string + index_position;
        char *destination_of_substring = position_of_insertion + length_of_insertion_string;
        const sl_string_size_t length_to_be_moved =
            sl_string__get_length(string) - index_position + 1U; // Plus 1 for the null term

        memmove(destination_of_substring, position_of_insertion, length_to_be_moved);
        strncpy(position_of_insertion, insert, length_of_insertion_string);

        result = true;
      }
    }
  }

  return result;
}

bool sl_string__append(sl_string_t string, const char *append) {
  bool result = false;

  if (NULL != append) {
    const sl_string_size_t length_of_original_string = sl_string__get_length(string);
    const sl_string_size_t length_of_insertion_string = (sl_string_size_t)strlen(append);
    const sl_string_size_t total_length = length_of_original_string + length_of_insertion_string;

    if (total_length <= sl_string__get_capacity(string)) {
      const sl_string_size_t length_to_copy_including_null = (1U + length_of_insertion_string);
      memcpy((string + length_of_original_string), append, length_to_copy_including_null);
      result = true;
    }
  }

  return result;
}

bool sl_string__append_char(sl_string_t string, char append) {
  const char string_to_append[] = {append, '\0'};
  return sl_string__append(string, string_to_append);
}

size_t sl_string__copy_to(const sl_string_t string, char *copy_to, size_t copy_to_max_size) {
  const size_t len = sl_string__get_length(string);
  const size_t bytes_to_copy_excluding_null = (copy_to_max_size > 0) ? (copy_to_max_size - 1) : 0;
  const size_t bytes_to_copy = (len < bytes_to_copy_excluding_null) ? len : bytes_to_copy_excluding_null;

  if (NULL != copy_to) {
    memcpy(copy_to, string, bytes_to_copy);
    copy_to[bytes_to_copy] = '\0';
  }

  return bytes_to_copy;
}

bool sl_string__equals_to(const sl_string_t string, const char *compare_with) {
  bool equal = false;

  if (NULL != compare_with) {
    equal = (0 == strcmp(string, compare_with));
  }

  return equal;
}

bool sl_string__equals_to_ignore_case(const sl_string_t string, const char *compare_with_case_ignored) {
  bool equal = false;

  if (NULL != compare_with_case_ignored) {
    equal = (0 == strcasecmp(string, compare_with_case_ignored));
  }

  return equal;
}

int sl_string__last_index_of(const sl_string_t string, const char *index_of) {
  const char *pointer_to_last = NULL;

  if (NULL != index_of) {
    const char *pointer_to_first_in_substring = strstr(string, index_of);
    const size_t substring_length = strlen(index_of);

    while (NULL != pointer_to_first_in_substring) {
      pointer_to_last = pointer_to_first_in_substring;
      pointer_to_first_in_substring = strstr(pointer_to_first_in_substring + substring_length, index_of);
    }
  }

  return ((NULL != pointer_to_last) ? (int)sl_utils__pointer_distance(pointer_to_last, string) : sl_string_error);
}

int sl_string__last_index_of_ignore_case(const sl_string_t string, const char *index_of_case_ignored) {
  int index_of_string = sl_string_error;

  if (NULL != index_of_case_ignored) {
    const char *pointer_to_first_char_in_substring = strchr(string, tolower((int)(*index_of_case_ignored)));
    if (NULL == pointer_to_first_char_in_substring) {
      pointer_to_first_char_in_substring = strchr(string, toupper((int)(*index_of_case_ignored)));
    }

    const size_t length_of_string_to_find = strlen(index_of_case_ignored);
    while (NULL != pointer_to_first_char_in_substring) {
      size_t num_of_chars_to_move_substring_start_location = 1;

      if (0 == strncasecmp(pointer_to_first_char_in_substring, index_of_case_ignored, length_of_string_to_find)) {
        index_of_string = (int)sl_utils__pointer_distance(pointer_to_first_char_in_substring, string);
        num_of_chars_to_move_substring_start_location = length_of_string_to_find;
      }

      const char *pointer_to_last_char = pointer_to_first_char_in_substring;
      pointer_to_first_char_in_substring = strchr(pointer_to_last_char + num_of_chars_to_move_substring_start_location,
                                                  tolower((int)(*index_of_case_ignored)));

      if (NULL == pointer_to_first_char_in_substring) {
        pointer_to_first_char_in_substring =
            strchr(pointer_to_last_char + num_of_chars_to_move_substring_start_location,
                   toupper((int)(*index_of_case_ignored)));
      }
    }
  }

  return index_of_string;
}

int sl_string__first_index_of(const sl_string_t string, const char *index_of) {
  const char *pointer_to_first = NULL;

  if (NULL != index_of) {
    pointer_to_first = strstr(string, index_of);
  }

  return ((NULL != pointer_to_first) ? (int)sl_utils__pointer_distance(pointer_to_first, string) : sl_string_error);
}

int sl_string__first_index_of_ignore_case(const sl_string_t string, const char *index_of_case_ignored) {
  int index_of_string = sl_string_error;

  if (NULL != index_of_case_ignored) {
    const char *pointer_to_first_char_in_substring = strchr(string, tolower((int)(*index_of_case_ignored)));
    if (NULL == pointer_to_first_char_in_substring) {
      pointer_to_first_char_in_substring = strchr(string, toupper((int)(*index_of_case_ignored)));
    }

    const size_t length_of_index_of_case_ignored = strlen(index_of_case_ignored);
    while (NULL != pointer_to_first_char_in_substring) {
      size_t num_of_chars_to_move_substring_start_location = 1;

      if (0 ==
          strncasecmp(pointer_to_first_char_in_substring, index_of_case_ignored, length_of_index_of_case_ignored)) {
        index_of_string = (int)sl_utils__pointer_distance(pointer_to_first_char_in_substring, string);
        break;
      }

      const char *first_char_in_substring_storage_ptr = pointer_to_first_char_in_substring;
      pointer_to_first_char_in_substring =
          strchr(first_char_in_substring_storage_ptr + num_of_chars_to_move_substring_start_location,
                 tolower((int)(*index_of_case_ignored)));

      if (NULL == pointer_to_first_char_in_substring) {
        pointer_to_first_char_in_substring =
            strchr(first_char_in_substring_storage_ptr + num_of_chars_to_move_substring_start_location,
                   toupper((int)(*index_of_case_ignored)));
      }
    }
  }

  return index_of_string;
}

bool sl_string__contains(const sl_string_t string, const char *substring) {
  return (sl_string_error != sl_string__first_index_of(string, substring));
}

bool sl_string__contains_ignore_case(const sl_string_t string, const char *substring_case_ignored) {
  return (sl_string_error != sl_string__first_index_of_ignore_case(string, substring_case_ignored));
}

int sl_string__count_of(const sl_string_t string, const char *count_of) {
  int count = sl_string_error;

  if (NULL != count_of) {
    count = 0;
    const char *pointer_to_first_in_substring = strstr(string, count_of);
    const size_t length_of_count_of_string = strlen(count_of);
    while (NULL != pointer_to_first_in_substring) {
      count++;
      pointer_to_first_in_substring = strstr(pointer_to_first_in_substring + length_of_count_of_string, count_of);
    }
  }

  return count;
}

bool sl_string__begins_with(const sl_string_t string, const char *begin_with) {
  const size_t substring_length = (NULL == begin_with) ? 0U : strlen(begin_with);
  return sl_string__begins_with_limited_chars(string, begin_with, substring_length);
}

bool sl_string__begins_with_limited_chars(const sl_string_t string, const char *begin_with,
                                          size_t max_chars_of_begin_with_to_compare) {
  bool result = false;

  if (NULL != begin_with) {
    const sl_string_size_t string_length = sl_string__get_length(string);

    // strncmp/strncasecmp are used here instead of strstr as it
    // allows to check only the beginning of the string
    if (string_length >= max_chars_of_begin_with_to_compare) {
      if (0 == strncmp(string, begin_with, max_chars_of_begin_with_to_compare)) {
        result = true;
      }
    }
  }

  return result;
}

bool sl_string__begins_with_ignore_case(const sl_string_t string, const char *begin_with_case_ignored) {
  bool result = false;

  if (NULL != begin_with_case_ignored) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t substring_length = strlen(begin_with_case_ignored);

    // strncmp/strncasecmp are used here instead of strstr as it
    // allows to check only the beginning of the string
    if (string_length >= substring_length) {
      if (0 == strncasecmp(string, begin_with_case_ignored, substring_length)) {
        result = true;
      }
    }
  }

  return result;
}

bool sl_string__begins_with_whole_word(const sl_string_t string, const char *begin_with, const char *delimiter_string) {
  bool result = false;

  if ((NULL != begin_with) && (NULL != delimiter_string)) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t substring_length = strlen(begin_with);

    // strncmp/strncasecmp are used here instead of strstr as it
    // allows to check only the beginning of the string
    if (string_length >= substring_length) {
      if (0 == strncmp(string, begin_with, substring_length)) {
        result = ((*delimiter_string == string[substring_length]) || ('\0' == string[substring_length]));
      }
    }
  }

  return result;
}

bool sl_string__begins_with_whole_word_ignore_case(const sl_string_t string, const char *begin_with_case_ignored,
                                                   const char *delimiter_string) {
  bool result = false;

  if ((NULL != begin_with_case_ignored) && (NULL != delimiter_string)) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t substring_length = strlen(begin_with_case_ignored);

    if (string_length >= substring_length) {
      if (0 == strncasecmp(string, begin_with_case_ignored, substring_length)) {
        result = ((*delimiter_string == string[substring_length]) || ('\0' == *delimiter_string));
      }
    }
  }

  return result;
}

bool sl_string__ends_with(const sl_string_t string, const char *ends_with) {
  bool result = false;

  if (NULL != ends_with) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t substring_length = strlen(ends_with);

    if (string_length >= substring_length) {
      if (0 == strncmp(string + string_length - substring_length, ends_with, substring_length)) {
        result = true;
      }
    }
  }

  return result;
}

bool sl_string__ends_with_ignore_case(const sl_string_t string, const char *ends_with_case_ignored) {
  bool result = false;

  if (NULL != ends_with_case_ignored) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t substring_length = strlen(ends_with_case_ignored);

    if (string_length >= substring_length) {
      if (0 == strncasecmp(string + string_length - substring_length, ends_with_case_ignored, substring_length)) {
        result = true;
      }
    }
  }

  return result;
}

bool sl_string__ends_with_newline(const sl_string_t string) {
  bool status = false;
  const sl_string_size_t length = sl_string__get_length(string);

  if (length > 0U) {
    const sl_string_size_t last_index = length - 1U;
    const char last_char = string[last_index];
    status = ('\r' == last_char) || ('\n' == last_char);
  }

  return status;
}

bool sl_string__erase(sl_string_t string, const char *erase) {
  bool result = false;

  if (NULL != erase) {
    const int index_to_erase = sl_string__first_index_of(string, erase);
    if (sl_string_error != index_to_erase) {
      result = sl_string__erase_after(string, (sl_string_size_t)index_to_erase, (sl_string_size_t)strlen(erase));
    }
  }

  return result;
}

bool sl_string__erase_first(sl_string_t string, sl_string_size_t n_chars) {
  return sl_string__erase_after(string, 0, n_chars);
}

bool sl_string__erase_last(sl_string_t string, sl_string_size_t n_chars) {
  const sl_string_size_t string_length = sl_string__get_length(string);
  const sl_string_size_t start_of_erase_index =
      (n_chars > string_length) ? 0U : (sl_string_size_t)(string_length - n_chars);

  return sl_string__erase_after(string, start_of_erase_index, n_chars);
}

bool sl_string__erase_at(sl_string_t string, sl_string_size_t index_position) {
  return sl_string__erase_after(string, index_position, 1);
}

bool sl_string__erase_after(sl_string_t string, sl_string_size_t erase_index_position, sl_string_size_t n_chars) {
  const sl_string_size_t string_length = sl_string__get_length(string);
  bool result = false;

  if (erase_index_position < string_length) {
    n_chars = (n_chars > (string_length - erase_index_position)) ? (string_length - erase_index_position) : n_chars;
    const sl_string_size_t num_chars_to_move = string_length - erase_index_position - n_chars + 1U;

    memmove(string + erase_index_position, string + erase_index_position + n_chars, num_chars_to_move);
    result = true;
  }

  return result;
}

bool sl_string__erase_at_substring(sl_string_t string, const char *erase_at_substring) {
  const char *found = strstr(string, erase_at_substring);
  const bool found_something_to_erase = (NULL != found);

  if (found_something_to_erase) {
    const sl_string_size_t position = (sl_string_size_t)sl_utils__pointer_distance(found, string);
    string[position] = '\0';
  }

  return found_something_to_erase;
}

bool sl_string__erase_first_word(sl_string_t string, char word_separater) {
  bool result = false;
  const char *word_ptr = strchr(string, (int)word_separater);

  if (NULL != word_ptr) {
    const sl_string_size_t n_chars = (sl_string_size_t)sl_utils__pointer_distance(word_ptr, string) + 1U;
    result = sl_string__erase_first(string, n_chars); // Plus 1 to also erase the word_seperator
  }

  return result;
}

sl_string_size_t sl_string__erase_special_chars(sl_string_t string) {
  sl_string_size_t chars_removed = 0;
  const sl_string_size_t string_length = sl_string__get_length(string);

  for (sl_string_size_t character = 0; character < string_length; character++) {
    if (0U == isalnum((uint8_t)string[character])) {
      if (!sl_string__erase_at(string, (sl_string_size_t)character)) {
        break;
      }
      character--;
      chars_removed++;
    }
  }

  return chars_removed;
}

bool sl_string__erase_int(sl_string_t string, int *erased_int) {
  bool parsed = false;

  const char *c = string;
  while (('\0' != *c) && (0U == isdigit((int)*c))) {
    ++c; // Skip chars until a digit
  }

  if (('\0' != *c) && (NULL != erased_int) && (0U != isdigit((int)*c))) {
    *erased_int = atoi(c);
    parsed = true;
  }

  while (('\0' != *c) && (0U != isdigit((int)*c))) {
    ++c; // Skip the digits we processed in atoi() above
  }

  // Erase the integer we processed above
  const sl_string_size_t chars_to_erase = (sl_string_size_t)sl_utils__pointer_distance(c, string);
  (void)sl_string__erase_first(string, chars_to_erase);

  return parsed;
}

bool sl_string__trim_end(sl_string_t string, const char *chars_to_trim) {
  bool result = false;

  if (NULL != chars_to_trim) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t trim_char_length = strlen(chars_to_trim);

    for (int character = ((int)string_length - 1); character >= 0; character--) {
      bool trim_done = false;

      for (size_t trim_char = 0; trim_char < trim_char_length; trim_char++) {
        if (chars_to_trim[trim_char] == string[character]) {
          string[character] = '\0';
          trim_done = true;
          break;
        }
      }

      if (!trim_done) {
        break;
      }
    }
    result = true;
  }

  return result;
}

bool sl_string__trim_start(sl_string_t string, const char *chars_to_trim) {
  bool result = false;
  sl_string_size_t n_chars = 0;

  if (NULL != chars_to_trim) {
    const sl_string_size_t string_length = sl_string__get_length(string);
    const size_t trim_char_length = strlen(chars_to_trim);

    for (; n_chars < string_length; n_chars++) {
      bool trim_done = false;

      for (size_t trim_char = 0; trim_char < trim_char_length; trim_char++) {
        if (chars_to_trim[trim_char] == string[n_chars]) {
          trim_done = true;
          break;
        }
      }

      if (!trim_done) {
        result = sl_string__erase_first(string, n_chars);
        break;
      }
    }
  }

  return result;
}

bool sl_string__replace_first(sl_string_t string, const char *replace, const char *replace_with) {
  bool result = false;

  if ((NULL != replace) && (NULL != replace_with)) {
    const int start_of_replace_index = sl_string__first_index_of(string, replace);
    result = true;

    if (sl_string_error != start_of_replace_index) {
      (void)sl_string__erase_after(string, (sl_string_size_t)start_of_replace_index, (sl_string_size_t)strlen(replace));
      result = sl_string__insert_at(string, (uint16_t)start_of_replace_index, replace_with);
    }
  }

  return result;
}

bool sl_string__replace_last(sl_string_t string, const char *replace, const char *replace_with) {
  bool result = false;

  if ((NULL != replace) && (NULL != replace_with)) {
    const int start_of_replace_index = sl_string__last_index_of(string, replace);
    result = true;

    if (sl_string_error != start_of_replace_index) {
      (void)sl_string__erase_after(string, (sl_string_size_t)start_of_replace_index, (sl_string_size_t)strlen(replace));
      result = sl_string__insert_at(string, (sl_string_size_t)start_of_replace_index, replace_with);
    }
  }

  return result;
}

int sl_string__replace_all(sl_string_t string, const char *replace, const char *replace_with) {
  int num_replaced = sl_string_error;

  if ((NULL != replace) && (NULL != replace_with)) {
    int start_of_replace_index = sl_string__first_index_of(string, replace);
    num_replaced = 0;

    const sl_string_size_t length_of_replace = (sl_string_size_t)strlen(replace);
    while (sl_string_error != start_of_replace_index) {
      bool result = sl_string__erase_after(string, (sl_string_size_t)start_of_replace_index, length_of_replace);
      if (result) {
        (void)sl_string__insert_at(string, (sl_string_size_t)start_of_replace_index, replace_with);
      }

      start_of_replace_index = sl_string__first_index_of(string, replace);
      num_replaced++;
    }
  }

  return num_replaced;
}
