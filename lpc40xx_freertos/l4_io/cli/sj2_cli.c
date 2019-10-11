#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app_cli.h"
#include "cli_handlers.h"
#include "common_macros.h"

/// Output all CLI to the standard output
static void sj2_cli__output_function(app_cli__argument_t argument, const char *string);
static void sj2_cli__task(void *p);
static void sj2_cli__get_line(sl_string_t input_line);
static void sj2_cli__handle_backspace_logic(sl_string_t input_line, char input_byte);

void sj2_cli__init(void) {
  // Note: Some terminals do not support color output so you may see wierd chars like '[34m'
  const bool color_output = true;
  const char *separator = "--------------------------------------------------------------------------------\r\n";

  static app_cli_s sj2_cli_struct;
  sj2_cli_struct = app_cli__initialize(4, sj2_cli__output_function, !color_output, separator);

  // Need static struct that does not go out of scope
  static app_cli__command_s hello_command = {.command_name = "hello",
                                             .help_message_for_command = "responds back with 'hello world'",
                                             .app_cli_handler = cli__hello};
  static app_cli__command_s task_list = {.command_name = "tasklist",
                                         .help_message_for_command =
                                             "Outputs list of RTOS tasks, CPU and stack usage. 'tasklist <time>' will "
                                             "display CPU utilization within this time window.",
                                         .app_cli_handler = cli__task_list};

  // Add your CLI commands in sorted order
  app_cli__add_command_handler(&sj2_cli_struct, &hello_command);
  app_cli__add_command_handler(&sj2_cli_struct, &task_list);

  // In case other tasks are hogging the CPU, it would be useful to run the CLI
  // at high priority to at least be able to see what is going on
  static StackType_t task_stack[512];
  static StaticTask_t task_struct;
  xTaskCreateStatic(sj2_cli__task, "cli", ARRAY_SIZE(task_stack), &sj2_cli_struct, PRIORITY_HIGH, task_stack,
                    &task_struct);
}

static void sj2_cli__output_function(app_cli__argument_t argument, const char *string) {
  while (*string != '\0') {
    putchar(*string);
    ++string;
  }
}

static void sj2_cli__task(void *task_parameter) {
  app_cli_s *sj2_cli_struct = (app_cli_s *)task_parameter;
  void *unused_cli_param = NULL;
  char string_memory[256];

  // Start by greeting the use with the 'help' command
  sl_string_t user_input = sl_string__initialize_from(string_memory, sizeof(string_memory), "help");

  while (true) {
    app_cli__process_input(sj2_cli_struct, unused_cli_param, user_input);
    sj2_cli__get_line(user_input);
  }
}

static void sj2_cli__get_line(sl_string_t input_line) {
  sl_string__clear(input_line);

  // As long as user does not enter a full line, continue to get input
  while (!sl_string__ends_with_newline(input_line)) {
    const char input_byte = getchar();
    sj2_cli__handle_backspace_logic(input_line, input_byte);

    if (sl_string__is_full(input_line)) {
      break;
    }
  }
}

static void sj2_cli__handle_backspace_logic(sl_string_t input_line, char input_byte) {
  const char backspace = '\b';

  if (backspace == input_byte) {
    sl_string__erase_last(input_line, 1);
    putchar(backspace);
    putchar(' ');
    putchar(backspace);
  } else {
    putchar(input_byte);
    sl_string__append_char(input_line, input_byte);
  }
}