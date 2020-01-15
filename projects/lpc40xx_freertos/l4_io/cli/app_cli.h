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

/**
 * @file CLI module to handling commands and giving back responses
 *
 @code

// You can use the 'argument' to possible route output messages to different output interfaces
static void app_cli_output_function(app_cli__argument_t argument, const char *string) {
  printf("%s", string);
}

static app_cli_status_e app_cli__hello_handler(app_cli__argument_t argument,
                                               sl_string_t user_input_minus_command_name,
                                               app_cli__print_string_function output_print_string) {
  sl_string_t output_string = user_input_minus_command_name;  // re-use the string

  sl_string__printf(output_string, "hello world!\n");
  output_print_string(argument, output_string);

  return APP_CLI_STATUS__SUCCESS;
}

static void cli_example(void) {
  app_cli_s cli = app_cli__initialize(4, app_cli_output_function);

  static app_cli__command_s hello_command = {.command_name = "hello",
                                             .help_message_for_command = "responds back with 'hello world'",
                                             .app_cli_handler = app_cli__hello_handler};

  app_cli__add_command_handler(&cli, &hello_command);

  app_cli__process_input(&cli, NULL, "hello");

  // Help command is built-in which will print the list of all commands
  app_cli__process_input(&cli, NULL, "help");
}

 @endcode
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_string.h"

/*******************************************************************************
 *
 *                               D E F I N E S
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *                                 E N U M S
 *
 ******************************************************************************/
typedef enum {
  APP_CLI_STATUS__SUCCESS = 0,
  APP_CLI_STATUS__HANDLER_FAILURE,
} app_cli_status_e;

/*******************************************************************************
 *
 *                               T Y P E D E F S
 *
 ******************************************************************************/

/**
 * Argument type for the CLI handlers
 */
typedef void *app_cli__argument_t;

/**
 * The CLI print function
 * @param argument This is the same argument coming down from app_cli__process_input()
 * @param string This string should be output back to the source that sent the CLI command
 */
typedef void (*app_cli__print_string_function)(app_cli__argument_t argument, const char *string);

/**
 * A command_name is composed of its name for the command_name and its help
 * Since the commands is stored as a linked list, we also have the 'pointer_of_next_command' link
 */
typedef struct app_cli__command_s {
  const char *command_name;
  const char *help_message_for_command;

  /**
   * @param argument This is the same argument coming down from app_cli__process_input()
   *
   * @param user_input_minus_command_name
   * A CLI handler function receives the command arguments. For example, if the command is "rf foo bar"
   * then the command_arguments is the string with the command name stripped, so "foo bar"
   *
   * @param output_function The output is the interface that should be used to provide response
   *
   * @returns true if command was successful, otherwise the CLI will print an error for you
   */
  app_cli_status_e (*app_cli_handler)(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                      app_cli__print_string_function output_function);

  struct app_cli__command_s *pointer_of_next_command; ///< User should not use this member
} app_cli__command_s;

/**
 * Instance of the CLI
 */
typedef struct {
  app_cli__command_s *commands_head_pointer;
  size_t minimum_command_chars_to_match;

  app_cli__print_string_function output_function; ///< This function is used to output CLI's response
  const char *terminal_string;
} app_cli_s;

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

/**
 * CLI is initialized by providing an output interface
 *
 * @param minimum_command_chars_to_match
 * If the full command name does not yield a handler, then 'short commands' matcher is used
 * For example, if the command is 'memory' and user types 'mem' with this parameter being
 * the value of 3, then the CLI will match the short command with its handler. Note that this
 * only occurs when the full command-name did not match any registered handler.
 *
 * @param output_print_string
 * This function is used to output CLI data; it is also given to each CLI handler to output its data
 *
 * @param terminal_string
 * Typyically this can be dashed line to separate CLI output, and it can also include ASCII terminal chars (ETX, EOT)
 */
app_cli_s app_cli__initialize(size_t minimum_command_chars_to_match, app_cli__print_string_function output_function,
                              const char *terminal_string);

/**
 * @param app_cli_command_static_memory A handler for the CLI; this memory should not go out of scope
 */
void app_cli__add_command_handler(app_cli_s *cli, app_cli__command_s *app_cli_command_static_memory);

/**
 * Processes an input command from the user
 *
 * @post CLI will match the input command with one of its handler, and invoke its callback
 * Otherwise CLI will return without calling and functions and outputs an error message
 *
 * @param cli_argument
 * This argument is not used by the CLI itself and it is a pass-through argument to CLI handlers
 * and to app_cli__print_string_function. It can be used to identify input source and output
 * the response to different channels, such as UART0 vs. UART1
 *
 * @param input_string The design of the function is such that it allows the user to buffer the
 * input string from any input source, and the CLI module is agnostic to where the data came from
 *
 * @note The input_string parameter is re-used, manipulated, and is basically destroyed ...
 * Since this string is re-used for output, CLI itself needs around 128 bytes, but if one of the
 * CLI handlers also use re-use it, then size it according to your requirement
 */
void app_cli__process_input(app_cli_s *cli, app_cli__argument_t cli_argument, sl_string_t input_string);

#ifdef __cplusplus
} /* extern "C" */
#endif
