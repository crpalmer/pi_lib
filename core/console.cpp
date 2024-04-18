#include <stdio.h>
#include "pi.h"
#include "console.h"
#include "consoles.h"

void Console::process_cmd(const char *cmd) {
    size_t n = strlen(cmd);
    if (is_command(cmd, "bootsel")) {
	pi_reboot_bootloader();
    } else if (is_command(cmd, "?") || is_command(cmd, "help")) {
	usage();
    } else {
	consoles_write_str("Invalid command, type ? or help for help.\n");
    }
}

void Console::usage() {
    consoles_write_str("Usage:\n\nbootsel - reboot to bootloader.\n");
}
