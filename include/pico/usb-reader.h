#ifndef __USB_READER_H__
#define __USB_READER_H__

#include "reader.h"

class USBReader : public Reader {
public:
    USBReader(size_t buf_size = 256);
    const char *readline() override;

private:
    char  *buf;
    size_t a_buf;
};

#endif
