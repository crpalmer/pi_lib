#ifndef __HTTPD_FILESYSTEM_HANDLER_H__
#define __HTTPD_FILESYSTEM_HANDLER_H__

#include <string>
#include "httpd-server.h"

class HttpdFilesystemHandler : public HttpdPrefixHandler {
public:
    HttpdFilesystemHandler(std::string path) : path(path) {
    }

    HttpdResponse *open(std::string relative_path) override;

private:
    std::string path;
};

#endif
