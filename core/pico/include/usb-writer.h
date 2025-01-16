#ifndef __USB_WRITER_H__
#define __USB_WRITER_H__

#include "writer.h"

class USBWriter : public Writer {
public:
    USBWriter();
    int write_str(const char *s) override;
};

#endif
