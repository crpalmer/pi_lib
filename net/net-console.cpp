#include "net-console.h"

NetConsole::~NetConsole() {
    if (fd >= 0) closesocket(fd);
}
