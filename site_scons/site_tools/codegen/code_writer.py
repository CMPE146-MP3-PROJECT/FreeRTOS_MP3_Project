from collections import defaultdict
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "site_packages"))

import cantools

"""
References:
    https://cantools.readthedocs.io/en/latest/#cantools.database.can.Message
    https://cantools.readthedocs.io/en/latest/#cantools.database.can.Signal
"""

ENUM_SUFFIX = "_e"
GENERATE_ALL_NODE_NAME = "ALL"


class InvalidDBCNodeError(Exception):
    pass


class CodeWriter(object):
    def __init__(self, dbc_filepath, dbc_node_name):
        self._stream = StringIO()
        self._dbc_node_name = dbc_node_name

        self._dbc_filepath = dbc_filepath
        self._dbc = cantools.database.load_file(dbc_filepath)
        self._valid_node_names = list(map(lambda node: node.name, self._dbc.nodes))

        if (self._dbc_node_name != GENERATE_ALL_NODE_NAME) and (self._dbc_node_name not in self._valid_node_names):
            raise InvalidDBCNodeError("Invalid node [{}]! Available nodes {}".format(self._dbc_node_name, self._valid_node_names))

        self._file_header()
        self._common_structs()
        self._generate_enums()
        self._message_header_instances()
        self._structs()
        self._mia_user_dependencies()
        self._encode_methods()
        self._decode_methods()
        self._mia_methods()
        self._write_footer()

    def __str__(self):
        return self._stream.getvalue()

    """
    Private methods
    """

    def _file_header(self):
        dbc_filename = os.path.basename(self._dbc_filepath)

        self._stream.write((
            "// clang-format off\n"
            "// AUTO-GENERATED - DO NOT EDIT\n"
            "// Generated from {0}\n"
            "#pragma once\n"
            "\n"
            "#include <stdbool.h>\n"
            "#include <stdint.h>\n"
            "#include <string.h>\n"
            "\n"
            "#ifndef MIN_OF\n"
            "#define MIN_OF(x,y) ((x) < (y) ? (x) : (y))\n"
            "#endif\n"
            "#ifndef MAX_OF\n"
            "#define MAX_OF(x,y) ((x) > (y) ? (x) : (y))\n"
            "#endif\n"
            "\n"
            "/**\n"
            " * Extern dependency to use dbc_encode_and_send_*() API\n"
            " * This is designed such that the generated code does not depend on your CAN driver\n"
            " * @param argument_from_dbc_encode_and_send is a pass through argument from dbc_encode_and_send_*()\n"
            " * @returns the value returned from is returned from dbc_encode_and_send_*()\n"
            " */\n"
            "extern bool dbc_send_can_message(void * argument_from_dbc_encode_and_send, uint32_t message_id, const uint8_t bytes[8], uint8_t dlc);\n"
            "\n"
        ).format(dbc_filename))

    def _write_footer(self):
        self._stream.write( "// clang-format off\n")

    def _common_structs(self):
        self._stream.write(
            "/// Missing in Action (MIA) structure\n"
            "typedef struct {\n"
            "  uint32_t mia_counter; ///< Counter used to track MIA\n"
            "} dbc_mia_info_t;\n"
            "\n"
            "/**\n"
            " * dbc_encode_*() API returns this instance to indicate message ID and DLC that should be used\n"
            " * dbc_decode_*() API checks against the provided header to make sure we are decoding the right message\n"
            " */\n"
            "typedef struct {\n"
            "  uint32_t message_id;  ///< CAN bus message identification\n"
            "  uint8_t message_dlc;  ///< Data Length Code of the CAN message\n"
            "} dbc_message_header_t;\n"
        )

    def _mia_user_dependencies(self):
        line = ("// " + "-"*(80-3) + "\n")

        self._stream.write("\n")
        self._stream.write(line)
        self._stream.write("// When a message's MIA counter reaches this value\n"
                           "// corresponding MIA replacements occur\n")
        self._stream.write(line)
        for message in self._dbc.messages:
            self._stream.write("extern const uint32_t dbc_mia_threshold_{0};\n".format(message.name))

        self._stream.write("\n")
        self._stream.write(line)
        self._stream.write("// User must define these externed instances in their code to use MIA functions\n")
        self._stream.write("// These are copied during dbc_service_mia_*() when message MIA timeout occurs\n")
        self._stream.write(line)
        for message in self._dbc.messages:
            suggested_mia_time = "" if message.cycle_time == 0 else (" // Suggested MIA threshold: (3*{0})".format(message.cycle_time))
            self._stream.write(("extern const dbc_{0}_s ".format(message.name)).ljust(40))
            self._stream.write("dbc_mia_replacement_{0};{1}\n".format(message.name, suggested_mia_time))

        self._stream.write("\n")

    def _generate_enums(self):
        code = ""
        choices = {}

        for message in self._dbc.messages:
            for signal in message.signals:
                if signal.choices is not None:
                    choices[signal.name] = signal.choices

        # signal_name is the key',
        # enum_list is:
        # OrderedDict([(2, 'DRIVER_HEARTBEAT_cmd_REBOOT'), (1, 'DRIVER_HEARTBEAT_cmd_SYNC'), (0, 'DRIVER_HEARTBEAT_cmd_NOOP')])
        for signal_name, enum_list in choices.items():
            values = ""
            for value, enum_item_name in enum_list.items():
                values += "  {0} = {1},\n".format(enum_item_name, value)

            code += (
                "\n"
                "// Enumeration for {0}\n"
                "typedef enum {{\n"
                "{1}"
                "}} {2}{3};\n"
            ).format(signal_name, values, signal_name, ENUM_SUFFIX)

        self._stream.write(code)

    def _message_header_instances(self):
        self._stream.write('\n// Message headers containing CAN message IDs and their DLCs; @see dbc_message_header_t\n')
        for message in self._dbc.messages:
            self._stream.write('static const dbc_message_header_t dbc_header_{0}'.format(message.name).ljust(80))
            self._stream.write(' = {{ {1}U, {2} }};\n'.format(
                message.name, str(message.frame_id).rjust(8), message.length))

    def _structs(self, generate_layout=False):
        for message in self._dbc.messages:
            message_layout = ("\n" + message.layout_string() if generate_layout else "")
            signal_members = self._generate_struct_signals(message)

            # MIA only makes sense for messages we receive, and not the messages we send
            mia = "  dbc_mia_info_t mia_info;\n"
            comment = "" if message.comment is None else ("\n *   - "+message.comment)
            cycle_time = "" if message.cycle_time == 0 else ("\n *   - Expected every {0} ms".format(message.cycle_time))

            self._stream.write((
                "\n"
                "/**\n"
                " * {0}: Sent by '{1}'{2}{3}{4}\n"
                " */\n"
                "typedef struct {{\n"
                "{5}"
                "\n"
                "{6}}} dbc_{0}_s;\n"
            ).format(message.name, message.senders[0], comment, message_layout, cycle_time, mia, signal_members))

    def _decode_methods(self):
        for message in self._dbc.messages:
            validation_check = (
                "\n"
                "  if ((header.message_id != dbc_header_{0}.message_id) || (header.message_dlc != dbc_header_{1}.message_dlc)) {{\n"
                "    return !success;\n"
                "  }}\n"
            ).format(message.name, message.name)

            self._stream.write((
                "\n"
                "/**\n"
                " * Decode received message {0}: Sent by {1}\n"
                " *   Provided a dbc_message_header_t, this will attempt to decode the received message and return true upon success\n"
                " */\n"
                "static inline bool dbc_decode_{0}(dbc_{0}_s *message, const dbc_message_header_t header, const uint8_t bytes[8]) {{\n"
                "  const bool success = true;\n"
                "{2}\n"
                "{3}\n"
                "\n"
                "  message->mia_info.mia_counter = 0;\n"
                "  return success;\n"
                "}}\n"
            ).format(message.name, message.senders[0], validation_check, self._get_decode_signals_code(message)))

        self._stream.write("\n")

    def _get_decode_signals_code(self, message):
        code = "  uint64_t raw = 0;\n"

        if message.is_multiplexed():
            code += "  // Multiplexed signals are not handled yet\n"
            code += "  (void)raw;\n"
        else:
            for signal in message.signals:
                if 'little' not in signal.byte_order:
                    code += '#error "Ooops, I do not know how to work with Big Endian signals"\n'
                else:
                    code += self._get_decode_signal_code(signal)
                    code += '\n'

        # Remove excessive newlines from the end
        return code.rstrip()

    def _get_decode_signal_code(self, signal, raw_sig_name="raw"):
        bit_pos = signal.start
        remaining = signal.length
        byte_num = int(signal.start / 8)
        bit_count = 0
        code = ''

        while remaining > 0:
            bits_in_this_byte = min(8 - (bit_pos % 8), remaining)

            s = ""
            s += "  {0} |= ((uint64_t)((bytes[{1}] >> {2})".format(raw_sig_name, byte_num, (bit_pos % 8))
            s += " & 0x{0})) << {1}".format(format(2 ** bits_in_this_byte - 1, '02x'), bit_count)
            s += "; // {0} bits from B{1}\n".format(bits_in_this_byte, bit_pos)

            # Optimize
            s = s.replace(" >> 0", "")
            s = s.replace(" << 0", "")
            s = s.replace(" & 0xff", "")

            code += s
            if bit_count == 0:
                code = code.replace("|=", " =")

            byte_num += 1
            bit_pos += bits_in_this_byte
            remaining -= bits_in_this_byte
            bit_count += bits_in_this_byte

        enum_cast = "";
        if self._is_signal_an_enum(signal):
            enum_cast = '({0})'.format(self._get_signal_type(signal))

        # If the signal is not defined as a signed, then we will use this code
        unsigned_code = "message->{0} = {1}(({2} * {3}f) + ({4}));\n".format(signal.name, enum_cast, raw_sig_name,
                                                                             signal.scale, signal.offset)

        if signal.is_signed:
            mask = "(1 << {0})".format((signal.length - 1))
            s = ""
            s += "  if ({0} & {1}) {{ // Check signed bit of the raw DBC signal and sign extend from 'raw'\n".format(raw_sig_name, mask)
            s += "    message->{0} = ".format(signal.name)

            # Create signed extended number by first getting a type similar to '(int16_t)-1'
            # The only corner case is that this will not work for float, but float should not
            # be listed as a signed number
            signed_max = "UINT32_MAX"
            s += "(((({0} << {1}) | {2}) * {3}f) + ({4}));\n".format(
                signed_max, str(signal.length - 1), raw_sig_name, str(signal.scale), signal.offset
            )

            s += ("  }} else {{\n"
                  "    {0}"
                  "  }}\n").format(unsigned_code)
        else:
            s = "  " + unsigned_code

        # Optimize
        s = s.replace(" + (0)", "")
        s = s.replace(" * 1f)", ")")
        code += s

        return code

    def _encode_methods(self):
        for message in self._dbc.messages:
            if not self._message_is_relevant(message):
                self._stream.write(("\n"
                                       "/**\n"
                                       " * {0}:\n"
                                       " *   Transmitter: '{1}' with message ID {2} composed of {3} bytes\n"
                                       " *   **Since you ({4}) are not the transmitter, this function is not generated for you**\n"
                                       " */\n"
                                       "// static inline dbc_message_header_t dbc_encode_{0}(uint8_t bytes[8], const dbc_{0}_s *message);\n"
                                   ).format(message.name, message.senders[0], message.frame_id, message.length, self._dbc_node_name.upper()))
            else:
                encode_code = self._get_encode_signals_code(message)

                self._stream.write((
                    "\n"
                    "/**\n"
                    " * Encode to transmit {0}:\n"
                    " *   Transmitter: '{1}' with message ID {2} composed of {3} bytes\n"
                    " */\n"
                    "static inline dbc_message_header_t dbc_encode_{0}(uint8_t bytes[8], const dbc_{0}_s *message) {{\n"
                    "{4}\n"
                    "\n"
                    "  return dbc_header_{0};\n"
                    "}}\n"
                ).format(message.name, message.senders[0], message.frame_id, message.length, encode_code))

                self._stream.write((
                    "\n"
                    "/// @see dbc_encode_{0}(); this is its variant to encode and call dbc_send_can_message() to send the message\n"
                    "static inline bool dbc_encode_and_send_{0}(void *argument_for_dbc_send_can_message, const dbc_{0}_s *message) {{\n"
                    "  uint8_t bytes[8];\n"
                    "  const dbc_message_header_t header = dbc_encode_{0}(bytes, message);\n"
                    "  return dbc_send_can_message(argument_for_dbc_send_can_message, header.message_id, bytes, header.message_dlc);\n"
                    "}}\n"
                ).format(message.name))

    def _get_encode_signals_code(self, message):
        code = "  uint64_t raw = 0;\n"\
               "  memset(bytes, 0, 8);\n"\
               "\n"

        if message.is_multiplexed():
            code += "  // Multiplexed signals are not handled yet\n"
            code += "  (void)raw;\n\n"
        else:
            for signal in message.signals:
                if 'little' not in signal.byte_order:
                    code += '#error "Ooops, I do not know how to work with Big Endian signals"\n'
                else:
                    code += self._get_encode_signal_code(signal, "raw")
                    code += '\n'

        # Remove excessive newlines from the end
        return code.rstrip()

    def _get_encode_signal_code(self, signal, raw_sig_name):
        # Compute binary value
        # Encode should subtract offset then divide
        # TODO: Might have to add -0.5 for a negative signal

        signal_type = self._get_signal_type(signal)
        signal_is_float = self._get_signal_type(signal) == "float"

        # Cast to int64_t or higher type may be required becuase we may have int8_t signal with
        # value of 100, with an offset of -200 which will roll over and produce incorrect result
        if signal_is_float:
            cast = ""
        else:
            cast = "(int64_t)"

        raw_sig_code = ""
        min_max_comment = ""
        if signal.minimum is None and signal.maximum is None:
            raw_sig_code = "  {0} = ((uint64_t)((({1}message->{2} - ({3})) / {4}f) + 0.5f))".format(
                raw_sig_name, cast, signal.name, signal.offset, signal.scale
            )
        else:
            min_max_comment = " within range of [{0} -> {1}]".format(signal.minimum, signal.maximum)

            # When using MIN_OF/MAX_OF macros, we need to use 'f' notation to explicitly use float rather than double
            minimum = str(signal.minimum) + 'f' if signal_is_float else str(signal.minimum)
            maximum = str(signal.maximum) + 'f' if signal_is_float else str(signal.maximum)
            minimum = minimum.replace("0f", "0.0f")
            maximum = maximum.replace("0f", "0.0f")

            raw_sig_code = "  {0} = ((uint64_t)(((MAX_OF(MIN_OF({1}message->{2},{3}),{4}) - ({5})) / {6}f) + 0.5f))".format(
                raw_sig_name, cast, signal.name, maximum, minimum, signal.offset, signal.scale
            )

        offset_string = ""
        if signal.offset != 0:
            offset_string = " and offset={0}".format(signal.offset)

        signal_comment = ""
        if signal.is_signed:
            signal_comment = "  // Encode to raw {0}-bit SIGNED signal with scale={1}{2}{3}\n".format(signal.length, signal.scale, offset_string, min_max_comment)
        else:
            signal_comment = "  // Encode to raw {0}-bit signal with scale={1}{2}{3}\n".format(signal.length, signal.scale, offset_string, min_max_comment)

        raw_extract = raw_sig_code + " & 0x{0};\n".format(format(2 ** signal.length - 1, '02x'))

        # Optimize
        raw_extract = raw_extract.replace(" - (0)", "")
        raw_extract = raw_extract.replace(" / 1f)", ")")
        if signal.scale == 1:
            raw_extract = raw_extract.replace(" + 0.5f", "")

        code = signal_comment + raw_extract

        # Stuff the raw data into individual bytes
        bit_pos = signal.start
        remaining = signal.length
        byte_num = int(signal.start / 8)

        while remaining > 0:
            bits_in_this_byte = min(8 - (bit_pos % 8), remaining)

            s = ""
            s += "  bytes[{0}] |= (((uint8_t)({1} >> {2})".format(byte_num, raw_sig_name,
                                                                  str(bit_pos - signal.start).rjust(2))
            s += " & 0x{0}) << {1})".format(format(2 ** bits_in_this_byte - 1, '02x'), str(bit_pos % 8))
            s += "; // {0} bits at B{1}\n".format(str(bits_in_this_byte), str(bit_pos))

            # Optimize
            s = s.replace(" >> 0", "")
            s = s.replace(" << 0", "     ")

            # Cannot optimize by removing 0xff just for code safety
            # s = s.replace(" & 0xff", "")

            code += s
            byte_num += 1

            bit_pos += bits_in_this_byte
            remaining -= bits_in_this_byte

        return code

    def _mia_methods(self):
        self._stream.write(
            "// Do not use this function\n"
            "static inline bool dbc_service_mia_for(dbc_mia_info_t *mia_info, const uint32_t increment_mia_by, const uint32_t threshold) {\n"
            "  bool message_just_entered_mia = false;\n"
            "\n"
            "  if (mia_info->mia_counter >= threshold) {\n"
            "    // Message is already MIA\n"
            "  } else {\n"
            "    mia_info->mia_counter += increment_mia_by;\n"
            "    message_just_entered_mia = (mia_info->mia_counter >= threshold);\n"
            "  }\n"
            "\n"
            "  return message_just_entered_mia;\n"
            "}\n"
            "\n"
        )

        for message in self._dbc.messages:
            self._stream.write("static inline bool dbc_service_mia_{0}(dbc_{0}_s *message, const uint32_t increment_mia_by) {{\n"
                               "  const bool message_just_entered_mia = dbc_service_mia_for(&(message->mia_info), increment_mia_by, dbc_mia_threshold_{0});\n"
                               "\n"
                               "  if (message_just_entered_mia) {{\n"
                               "    const dbc_mia_info_t previous_mia = message->mia_info;\n"
                               "    *message = dbc_mia_replacement_{0};\n"
                               "    message->mia_info = previous_mia;\n"
                               "  }}\n"
                               "\n"
                               "  return message_just_entered_mia;\n"
                               "}}\n"
                               "\n".format(message.name)
                               )

    def _generate_struct_signals(self, message):
        signals_string = ""

        if not message.is_multiplexed():
            for signal in message.signals:
                type_and_name = "{0} {1}".format(self._get_signal_type(signal), signal.name)

                comment = "" if signal.comment is None else signal.comment
                comment += "" if signal.unit is None else (" unit: " + signal.unit)
                if len(comment) > 0:
                    comment = (' // ' + comment).replace('  ', ' ')

                signals_string += "  {0};{1}\n".format(type_and_name, comment)
        else:
            # For a muxed message, first generate non-mux symbols (which includes the mux itself)
            for signal in message.signals:
                if signal.multiplexer_ids is None:
                    type_and_name = "{0} {1}".format(self._get_signal_type(signal), signal.name)
                    signals_string += "  {0}; // Non-muxed signal\n".format(type_and_name)

            # Create a dictionary of a list where the key is the mux symbol
            muxed_signals = self._get_muxed_signals(message)

            for mux_id in muxed_signals:
                signals_string += '\n'
                for signal in muxed_signals[mux_id]:
                    type_and_name = "{0} {1}".format(self._get_signal_type(signal), signal.name)
                    signals_string += "  {0}; // M{1}\n".format(type_and_name, mux_id)

        return signals_string

    def _get_muxed_signals(self, message):
        # Create a dictionary of a list where the key is the mux symbol
        muxed_signals = defaultdict(list)

        for signal in message.signals:
            if signal.multiplexer_ids is not None:
                muxed_signals[signal.multiplexer_ids[0]].append(signal)

        return muxed_signals

    def _get_signal_type(self, signal):
        signal_type = "float";

        if self._is_signal_an_enum(signal):
            signal_type = signal.name + ENUM_SUFFIX
        elif (signal.scale * 1.0).is_integer():
            max_value = (2 ** signal.length) * signal.scale
            if signal.is_signed:
                max_value *= 2

            signal_type = "uint32_t"
            if max_value <= 256:
                signal_type = "uint8_t"
            elif max_value <= 65536:
                signal_type = "uint16_t"

            # If the signal is signed, or the offset is negative, remove "u" to use "int" type.
            if signal.is_signed or signal.offset < 0:
                signal_type = signal_type[1:]
        else:
            signal_type = "float"

        return signal_type

    def _is_signal_an_enum(self, signal):
        return signal.choices is not None

    def _message_is_relevant(self, message):
        return (message.senders[0].upper() == self._dbc_node_name.upper()) or (GENERATE_ALL_NODE_NAME == self._dbc_node_name.upper())

    """
    Accessors
    """

    @property
    def dbc_filepath(self):
        return self._dbc_filepath

    @property
    def dbc_node_name(self):
        return self._dbc_node_name

    @property
    def valid_node_names(self):
        return self._valid_node_names
