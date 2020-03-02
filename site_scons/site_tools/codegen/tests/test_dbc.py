import sys

sys.path.insert(0, "../site_packages")

import cantools


def main():
    dbc_filepath = "test.dbc"
    dbc = cantools.database.load_file(dbc_filepath)

    print_messages_and_signals_objects(dbc)
    print("\n---\n\n")
    print_messages_and_signals_simplified(dbc)


def print_messages_and_signals_objects(dbc):
    for message_obj in dbc.messages:
        print(message_obj)

        for signal_obj in message_obj.signals:
            print(signal_obj)

        print("")


def print_messages_and_signals_simplified(dbc):
    for message_obj in dbc.messages:
        for signal_obj in message_obj.signals:
            print("{}.{} - Receivers {}".format(
                    message_obj.name, signal_obj.name, signal_obj.receivers
                )
            )

            if signal_obj.is_multiplexer:
                print("Multiplexer signals")
            else:
                print("Length [{}] Start [{}] Scale [{}] Offset [{}] Signed [{}]".format(
                        signal_obj.length, signal_obj.start, signal_obj.scale, signal_obj.offset, signal_obj.is_signed
                    )
                )
                print("Multiplexer ID [{}] and Signal [{}]".format(signal_obj.multiplexer_ids, signal_obj.multiplexer_signal))
                assert isinstance(signal_obj.multiplexer_ids, list) or (signal_obj.multiplexer_ids is None)
            print ("")


if __name__ == "__main__":
    sys.exit(main())
