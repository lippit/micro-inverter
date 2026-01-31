import os


def usb_to_stlink():
    dts_path = os.path.join(".", "zephyr", "boards", "owntech", "spin", "spin.dts")
    search_str = "zephyr,console = &cdc_acm_uart0;"
    replace_str = "zephyr,console = &lpuart1;"

    with open(dts_path, "r") as file:
        file_contents = file.read()

    if replace_str in file_contents:
        print(f"Console already set for STLink in '{dts_path}'.")
        return

    if search_str in file_contents:
        new_contents = file_contents.replace(search_str, replace_str)
        with open(dts_path, "w") as file:
            file.write(new_contents)
        print(f"Replacement of '{search_str}' with '{replace_str}' in '{dts_path}' completed.")
        return

    print(f"Neither '{search_str}' nor '{replace_str}' found in '{dts_path}'. No replacement performed.")


usb_to_stlink()
