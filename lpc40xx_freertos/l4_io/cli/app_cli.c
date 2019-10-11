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

#include "app_cli.h"

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
static const app_cli__command_s *app_cli__private_find_command(app_cli_s *cli, const sl_string_t input_string) {
  app_cli__command_s *iterator = cli->commands_head_pointer;
  while (NULL != iterator) {
    if (sl_string__begins_with_whole_word(input_string, iterator->command_name, " ")) {
      break;
    }

    iterator = iterator->pointer_of_next_command;
  }

  return iterator;
}

static const app_cli__command_s *app_cli__private_find_short_command(app_cli_s *cli, const sl_string_t input_string) {
  app_cli__command_s *iterator = cli->commands_head_pointer;

  while (NULL != iterator) {
    /**
     * If the command is "sibros 123", we are checking if this 'input_string' begins with our command name but
     * we only want to match a limited number of chars of this command (cli->minimum_command_chars_to_match)
     */
    if (sl_string__begins_with_limited_chars(input_string, iterator->command_name,
                                             cli->minimum_command_chars_to_match)) {
      break;
    }

    iterator = iterator->pointer_of_next_command;
  }

  return iterator;
}

static void app_cli__private_handle_command(app_cli_s *cli, const app_cli__command_s *command,
                                            app_cli__argument_t cli_argument, sl_string_t input_string) {
  (void)sl_string__erase_first_word(input_string, ' ');
  const app_cli_status_e command_status = command->app_cli_handler(cli_argument, input_string, cli->output_function);

  if (APP_CLI_STATUS__SUCCESS != command_status) {
    sl_string_t output_string = input_string; // re-use

    (void)sl_string__insert_at(output_string, 0, "Command failed: ");
    cli->output_function(cli_argument, output_string);

    (void)sl_string__printf(output_string, "\r\nCommand help message: %s\r\n", command->help_message_for_command);
    cli->output_function(cli_argument, output_string);
  }
}

static void app_cli__private_print_help(const app_cli_s *cli, app_cli__argument_t cli_argument, sl_string_t output) {
  (void)sl_string__printf(output, "\r\nList of commands: \r\n");
  cli->output_function(cli_argument, output);

  app_cli__command_s *iterator = cli->commands_head_pointer;
  while (NULL != iterator) {
    if (cli->color_output) {
      (void)sl_string__printf(output,
                              "\x1B"
                              "[34m%16s\x1B"
                              "[0m: %s\r\n",
                              iterator->command_name, iterator->help_message_for_command);
    } else {
      (void)sl_string__printf(output, "%16s: %s\r\n", iterator->command_name, iterator->help_message_for_command);
    }
    cli->output_function(cli_argument, output);

    iterator = iterator->pointer_of_next_command;
  }
}

static void app_cli__private_handle_unmatched_command(const app_cli_s *cli, app_cli__argument_t cli_argument,
                                                      sl_string_t input_string) {
  sl_string_t output_string = input_string; // re-use

  (void)sl_string__insert_at(output_string, 0, "Unable to match any registered CLI command for: ");
  cli->output_function(cli_argument, output_string);

  app_cli__private_print_help(cli, cli_argument, output_string);
}

static void app_cli__private_process_input(app_cli_s *cli, app_cli__argument_t cli_argument, sl_string_t input_string) {
  const app_cli__command_s *command = app_cli__private_find_command(cli, input_string);

  // If full command not matched, then try matching short command
  if (NULL == command) {
    command = app_cli__private_find_short_command(cli, input_string);
  }

  if (NULL != command) {
    app_cli__private_handle_command(cli, command, cli_argument, input_string);
  } else {
    app_cli__private_handle_unmatched_command(cli, cli_argument, input_string);
  }
}

/*******************************************************************************
 *
 *                      P U B L I C    F U N C T I O N S
 *
 ******************************************************************************/

app_cli_s app_cli__initialize(size_t minimum_command_chars_to_match, app_cli__print_string_function output_function,
                              bool color_output, const char *terminal_string) {
  static const size_t minimum_number_of_chars_to_match = 4;
  const size_t max = minimum_number_of_chars_to_match > minimum_command_chars_to_match
                         ? minimum_number_of_chars_to_match
                         : minimum_command_chars_to_match;

  app_cli_s cli = {
      .commands_head_pointer = NULL,
      .minimum_command_chars_to_match = max,
      .output_function = output_function,
      .color_output = color_output,
      .terminal_string = terminal_string,
  };

  return cli;
}

void app_cli__add_command_handler(app_cli_s *cli, app_cli__command_s *app_cli_command_static_memory) {
  if (NULL != app_cli_command_static_memory) {
    app_cli_command_static_memory->pointer_of_next_command = cli->commands_head_pointer;
    cli->commands_head_pointer = app_cli_command_static_memory;
  }
}

void app_cli__process_input(app_cli_s *cli, app_cli__argument_t cli_argument, sl_string_t input_string) {
  (void)sl_string__trim_end(input_string, "\r\n");
  (void)sl_string__trim_start(input_string, "\r\n");

  if (sl_string__equals_to_ignore_case(input_string, "help")) {
    app_cli__private_print_help(cli, cli_argument, input_string);
  } else {
    app_cli__private_process_input(cli, cli_argument, input_string);
  }

  // Send a terminator to signal to any receiver that we are done outputting all output from our handlers
  if (NULL != cli->terminal_string) {
    cli->output_function(cli_argument, cli->terminal_string);
  }
}
