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
  const char *separator = "--------------------------------------------------------------------------------\r\n";

  static app_cli_s sj2_cli_struct;
  sj2_cli_struct = app_cli__initialize(4, sj2_cli__output_function, separator);

  // Need static struct that does not go out of scope
  static app_cli__command_s crash = {.command_name = "crash",
                                     .help_message_for_command =
                                         "Deliberately crashes the system to demonstrate how to debug a crash",
                                     .app_cli_handler = cli__crash_me};
  static app_cli__command_s i2c = {.command_name = "i2c",
                                   .help_message_for_command = "i2c read 0xDD 0xRR <n>\n"
                                                               "i2c write 0xDD 0xRR <value> <value> ...",
                                   .app_cli_handler = cli__i2c};
  static app_cli__command_s task_list = {.command_name = "tasklist",
                                         .help_message_for_command =
                                             "Outputs list of RTOS tasks, CPU and stack usage.\n"
                                             "tasklist <time>' will display CPU utilization within this time window.",
                                         .app_cli_handler = cli__task_list};

  // Add your CLI commands in descending sorted order
  app_cli__add_command_handler(&sj2_cli_struct, &task_list);
  app_cli__add_command_handler(&sj2_cli_struct, &i2c);
  app_cli__add_command_handler(&sj2_cli_struct, &crash);

  // In case other tasks are hogging the CPU, it would be useful to run the CLI
  // at high priority to at least be able to see what is going on
  static StackType_t task_stack[2048 / sizeof(StackType_t)];
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
  char string_memory[128];

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
    if (sl_string__get_length(input_line) > 0) {
      sl_string__erase_last(input_line, 1);
      putchar(backspace);
      putchar(' ');
      putchar(backspace);
    }
  } else {
    putchar(input_byte);
    sl_string__append_char(input_line, input_byte);
  }
}