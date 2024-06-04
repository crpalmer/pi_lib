#include "pi.h"
#include "buffer.h"
#include "file.h"
#include "httpd-filesystem-handler.h"

HttpdResponse *HttpdFilesystemHandler::open(std::string rel_path) {
    std::string full_path = path + "/" + rel_path;
    Buffer *b = buffer_file_open(full_path);

    if (! b) {
	fprintf(stderr, "Failed to open: %s\n", full_path.c_str());
	return NULL;
    }

    return new HttpdResponse(b);
}
