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

#include <string.h>

#include "unity.h"

#include "sl_string.h"

// Module under test
#include "app_cli.c"

// /******************************************************************************
//  *
//  *                               D E F I N E S
//  *
//  *****************************************************************************/

// /*******************************************************************************
//  *
//  *                                 E N U M S
//  *
//  ******************************************************************************/

// /******************************************************************************
//  *
//  *                              T Y P E D E F S
//  *
//  *****************************************************************************/

// /******************************************************************************
//  *
//  *        P R I V A T E    F U N C T I O N    D E C L A R A T I O N S
//  *
//  *****************************************************************************/

// /******************************************************************************
//  *
//  *             P R I V A T E    D A T A    D E F I N I T I O N S
//  *
//  *****************************************************************************/
// static app_cli_s cli;
// static sl_string_t cli_output_string;
// static char cli_output_string_memory[1024];
// static sl_string_t cli_input_string;
// static char cli_input_string_memory[1024];
// static void *no_arg = NULL;
// static app_cli__argument_t hello_world_cmd_handler_arg;

// /******************************************************************************
//  *
//  *                     P R I V A T E    F U N C T I O N S
//  *
//  *****************************************************************************/
// static void app_cli_output_function(app_cli__argument_t argument, const char *string) {
//   UNUSED(argument);
//   sl_string__append(cli_output_string, string);
// }

// static app_cli_status_e app_cli__hello_handler(app_cli__argument_t argument, sl_string_t
// user_input_minus_command_name,
//                                                app_cli__print_string_function output_print_string) {
//   sl_string_t output_string = user_input_minus_command_name; // re-use the string

//   sl_string__printf(output_string, "hello world!\n");
//   output_print_string(argument, output_string);
//   hello_world_cmd_handler_arg = argument;

//   return APP_CLI_STATUS__SUCCESS;
// }

// static app_cli_status_e app_cli__fail_handler(app_cli__argument_t argument, sl_string_t
// user_input_minus_command_name,
//                                               app_cli__print_string_function output_print_string) {
//   UNUSED(argument);
//   UNUSED(user_input_minus_command_name);
//   UNUSED(output_print_string);

//   return APP_CLI_STATUS__HANDLER_FAILURE;
// }

// static app_cli_status_e app_cli__sibros_handler(app_cli__argument_t argument, sl_string_t
// user_input_minus_command_name,
//                                                 app_cli__print_string_function output_print_string) {
//   UNUSED(argument);
//   sl_string__insert_at(user_input_minus_command_name, 0, "string argument was: ");
//   output_print_string(argument, user_input_minus_command_name);

//   return APP_CLI_STATUS__SUCCESS;
// }

// static sl_string_t input_string(const char *string) {
//   sl_string__printf(cli_input_string, "%s", string);
//   return cli_input_string;
// }

// /******************************************************************************
//  *
//  *                T E S T    S E T U P    &    T E A R D O W N
//  *
//  *****************************************************************************/
// void setUp(void) {
//   cli = app_cli__initialize(4, app_cli_output_function, false, "\x03");
//   cli_output_string = sl_string__initialize(cli_output_string_memory, sizeof(cli_output_string_memory));
//   cli_input_string = sl_string__initialize(cli_input_string_memory, sizeof(cli_input_string_memory));

//   static app_cli__command_s hello_command = {.command_name = "hello",
//                                              .help_message_for_command = "responds back with 'hello world'",
//                                              .app_cli_handler = app_cli__hello_handler};
//   static app_cli__command_s fail_command = {.command_name = "fail",
//                                             .help_message_for_command = "always returns failure",
//                                             .app_cli_handler = app_cli__fail_handler};
//   static app_cli__command_s sibros_command = {.command_name = "sibros",
//                                               .help_message_for_command = "outputs command string arguments",
//                                               .app_cli_handler = app_cli__sibros_handler};

//   app_cli__add_command_handler(&cli, &hello_command);
//   app_cli__add_command_handler(&cli, &fail_command);
//   app_cli__add_command_handler(&cli, &sibros_command);

//   hello_world_cmd_handler_arg = NULL;
// }

// void tearDown(void) {}

// /******************************************************************************
//  *
//  *                                 T E S T S
//  *
//  *****************************************************************************/
// void test_error_cases(void) {
//   // Should not cause a runtime fault
//   app_cli__add_command_handler(&cli, NULL);
// }

// void test_minimum_short_command(void) {
//   cli = app_cli__initialize(2, app_cli_output_function, false, "");
//   TEST_ASSERT_EQUAL(4, cli.minimum_command_chars_to_match);
// }

// void test_terminal_output(void) {
//   cli = app_cli__initialize(4, app_cli_output_function, false, NULL);
//   app_cli__process_input(&cli, no_arg, input_string("help"));
//   TEST_ASSERT_TRUE(NULL == strrchr(cli_output_string, '\x03'));

//   cli = app_cli__initialize(4, app_cli_output_function, false, "");
//   app_cli__process_input(&cli, no_arg, input_string("help"));
//   TEST_ASSERT_TRUE(NULL == strrchr(cli_output_string, '\x03'));

//   cli = app_cli__initialize(4, app_cli_output_function, false, "\x03");
//   app_cli__process_input(&cli, no_arg, input_string("help"));
//   TEST_ASSERT_TRUE(NULL != strrchr(cli_output_string, '\x03'));
// }

// void test_color_output(void) {
//   cli.color_output = true;

//   app_cli__process_input(&cli, no_arg, input_string("unmatched"));
//   TEST_ASSERT_TRUE(NULL != strstr(cli_output_string, "List of commands"));
//   TEST_ASSERT_TRUE(NULL != strstr(cli_output_string, "outputs command string arguments"));
//   TEST_ASSERT_TRUE(NULL != strstr(cli_output_string, "always returns failure"));
//   TEST_ASSERT_TRUE(NULL != strstr(cli_output_string, "responds back with 'hello world'"));
// }

// void test_unmatched_command(void) {
//   static const char *expected_unmatched_command_output = "Unable to match any registered CLI command for:
//   unmatched\r\n"
//                                                          "List of commands: \r\n"
//                                                          "          sibros: outputs command string arguments\r\n"
//                                                          "            fail: always returns failure\r\n"
//                                                          "           hello: responds back with 'hello
//                                                          world'\r\n\x03";

//   app_cli__process_input(&cli, no_arg, input_string("unmatched"));
//   TEST_ASSERT_EQUAL_STRING(expected_unmatched_command_output, cli_output_string);
// }

// void test_help_command(void) {
//   const char *expected_help_command_output = "\r\nList of commands: \r\n"
//                                              "          sibros: outputs command string arguments\r\n"
//                                              "            fail: always returns failure\r\n"
//                                              "           hello: responds back with 'hello world'\r\n\x03";

//   app_cli__process_input(&cli, no_arg, input_string("help"));
//   TEST_ASSERT_EQUAL_STRING(expected_help_command_output, cli_output_string);

//   sl_string__clear_all_memory(cli_output_string);
//   app_cli__process_input(&cli, no_arg, input_string("HELP"));
//   TEST_ASSERT_EQUAL_STRING(expected_help_command_output, cli_output_string);
// }

// void test_argument_passing(void) {
//   app_cli__argument_t expected_argument = (void *)0xDEADBEEF;

//   app_cli__process_input(&cli, expected_argument, input_string("hello"));
//   TEST_ASSERT_EQUAL_STRING("hello world!\n\x03", cli_output_string);
//   TEST_ASSERT_EQUAL_PTR(expected_argument, hello_world_cmd_handler_arg);
// }

// void test_hello_world_command(void) {
//   app_cli__process_input(&cli, no_arg, input_string("hello"));
//   TEST_ASSERT_EQUAL_STRING("hello world!\n\x03", cli_output_string);
// }

// void test_hello_world_command_full_command(void) {
//   cli.minimum_command_chars_to_match = 9999;
//   app_cli__process_input(&cli, no_arg, input_string("hello"));
//   TEST_ASSERT_EQUAL_STRING("hello world!\n\x03", cli_output_string);
// }

// void test_hello_world_short_command(void) {
//   app_cli__process_input(&cli, no_arg, input_string("hell"));
//   TEST_ASSERT_EQUAL_STRING("hello world!\n\x03", cli_output_string);
// }

// void test_failed_command(void) {
//   const char *expected_fail_command_output =
//       "Command failed: fail\r\nCommand help message: always returns failure\r\n\x03";

//   app_cli__process_input(&cli, no_arg, input_string("fail"));
//   TEST_ASSERT_EQUAL_STRING(expected_fail_command_output, cli_output_string);
// }

// void test_sibros_command(void) {
//   app_cli__process_input(&cli, no_arg, input_string("sibros 123"));
//   TEST_ASSERT_EQUAL_STRING("string argument was: 123\x03", cli_output_string);
// }

// void test_sibros_command__strip_newline_at_end(void) {
//   app_cli__process_input(&cli, no_arg, input_string("sibros 123\r\n"));
//   TEST_ASSERT_EQUAL_STRING("string argument was: 123\x03", cli_output_string);
// }

// void test_sibros_short_command(void) {
//   app_cli__process_input(&cli, no_arg, input_string("sibr 123"));
//   TEST_ASSERT_EQUAL_STRING("string argument was: 123\x03", cli_output_string);

//   sl_string__clear_all_memory(cli_output_string);
//   app_cli__process_input(&cli, no_arg, input_string("sibro 123"));
//   TEST_ASSERT_EQUAL_STRING("string argument was: 123\x03", cli_output_string);
// }

// void test_sibros_short_command_with_higher_number_of_chars(void) {
//   static const char *expected_unmatched_command_output = "Unable to match any registered CLI command for: sibro
//   123\r\n"
//                                                          "List of commands: \r\n"
//                                                          "          sibros: outputs command string arguments\r\n"
//                                                          "            fail: always returns failure\r\n"
//                                                          "           hello: responds back with 'hello
//                                                          world'\r\n\x03";

//   cli.minimum_command_chars_to_match = 6;

//   app_cli__process_input(&cli, no_arg, input_string("sibro 123"));
//   TEST_ASSERT_EQUAL_STRING(expected_unmatched_command_output, cli_output_string);
// }
