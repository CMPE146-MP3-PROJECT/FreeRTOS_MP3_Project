#include "app_cli.h"

app_cli_status_e cli__uart3_transmit(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                     app_cli__print_string_function cli_output);

app_cli_status_e cli__crash_me(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output);

app_cli_status_e cli__i2c(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                          app_cli__print_string_function cli_output);

app_cli_status_e cli__task_list(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                app_cli__print_string_function cli_output);

app_cli_status_e cli__my_task_control(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                                      app_cli__print_string_function cli_output);

app_cli_status_e cli__mp3_play(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                               app_cli__print_string_function cli_output);