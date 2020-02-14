#include "cli_handlers.h"

#include "i2c.h"

/// The I2C bus we will use for this CLI
static const i2c_e i2c_bus = I2C__2;

static void cli__i2c_read(const sl_string_t command_params, app_cli__print_string_function cli_output);
static void cli__i2c_write(sl_string_t command_params, app_cli__print_string_function cli_output);

app_cli_status_e cli__i2c(app_cli__argument_t argument, sl_string_t user_input_minus_command_name,
                          app_cli__print_string_function cli_output) {
  app_cli_status_e command_status = APP_CLI_STATUS__HANDLER_FAILURE;

  const bool read = sl_string__begins_with_ignore_case(user_input_minus_command_name, "read");
  const bool write = sl_string__begins_with_ignore_case(user_input_minus_command_name, "write");
  sl_string__erase_first_word(user_input_minus_command_name, ' ');
  sl_string_t command_params = user_input_minus_command_name;

  // i2c read 0xDD 0xRR <n>
  if (read) {
    cli__i2c_read(command_params, cli_output);
    command_status = APP_CLI_STATUS__SUCCESS;
  }
  // i2c write 0xDD 0xRR <value> <value> ...
  else if (write) {
    cli__i2c_write(command_params, cli_output);
    command_status = APP_CLI_STATUS__SUCCESS;
  } else {
    command_status = APP_CLI_STATUS__HANDLER_FAILURE;
  }

  return command_status;
}

static void cli__i2c_read(const sl_string_t command_params, app_cli__print_string_function cli_output) {
  uint8_t buffer[256] = {0};
  void *unused_cli_argument = NULL;

  char string_memory[128];
  sl_string_t output = sl_string__initialize(string_memory, sizeof(string_memory));

  unsigned slave_address = 0, slave_register = 0, count = 1;

  if (sl_string__scanf(command_params, "0x%x 0x%x %u", &slave_address, &slave_register, &count) >= 2) {
    if (i2c__read_slave_data(i2c_bus, slave_address, slave_register, &buffer[0], count)) {
      sl_string__printf(output, "I2C Read of Slave 0x%02X\n", slave_address);
      cli_output(unused_cli_argument, output);

      for (size_t index = 0; index < count; index++) {
        sl_string__printf(output, "  0x%02X: 0x%02X (%d)\n", (slave_register + index), buffer[index], buffer[index]);
        cli_output(unused_cli_argument, output);
      }
    } else {
      sl_string__set(output, "I2C bus error during i2c__read_slave_data()\n");
      cli_output(unused_cli_argument, output);
    }
  } else {
    sl_string__set(output, "Command error: Read command should be 'i2c read 0xDD 0xRR <n>'\n");
    cli_output(unused_cli_argument, output);
  }
}

static void cli__i2c_write(sl_string_t command_params, app_cli__print_string_function cli_output) {
  unsigned slave_address = 0, slave_register = 0;
  void *unused_cli_argument = NULL;
  uint8_t buffer[256] = {0};

  char string_memory[128];
  sl_string_t output = sl_string__initialize(string_memory, sizeof(string_memory));

  // i2c write 0xDD 0xRR <value> <value> ...
  if (2 == sl_string__scanf(command_params, "0x%x 0x%x", &slave_address, &slave_register)) {
    sl_string__erase_first_word(command_params, ' ');
    sl_string__erase_first_word(command_params, ' ');

    // Store one or more write <value> parameters to buffer as specified by the user
    int value_to_write = 0;
    size_t count = 0;
    while (sl_string__erase_int(command_params, &value_to_write)) {
      buffer[count] = (uint8_t)value_to_write;
      ++count;
    }

    if (i2c__write_slave_data(i2c_bus, slave_address, slave_register, &buffer[0], count)) {
      sl_string__printf(output, "Wrote %u bytes to slave 0x%02X\n", count, slave_address);
      cli_output(unused_cli_argument, output);

      for (size_t index = 0; index < count; index++) {
        sl_string__printf(output, "  [%3d] = 0x%02X (%d)\n", (slave_register + index), buffer[index], buffer[index]);
        cli_output(unused_cli_argument, output);
      }
    } else {
      sl_string__set(output, "I2C bus error during i2c__write_slave_data()\n");
      cli_output(unused_cli_argument, output);
    }
  } else {
    sl_string__set(output, "Command error: Write command should be 'i2c write 0xDD 0xRR <value> <value> ...'\n");
    cli_output(unused_cli_argument, output);
  }
}