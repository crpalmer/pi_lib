#include <stdio.h>
#include "pi.h"
#include "console.h"

void Console::process_cmd(const char *cmd) {
    if (is_command(cmd, "bootsel")) {
	pi_reboot_bootloader();
    } else if (is_command(cmd, "?") || is_command(cmd, "help")) {
	usage();
    } else {
	write_str("Invalid command, type ? or help for help.\n");
    }
}

void Console::usage() {
    write_str("Usage:\n\nbootsel - reboot to bootloader.\n");
}
