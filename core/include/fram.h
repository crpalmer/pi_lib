#ifndef __FRAM_H__
#define __FRAM_H__

class FRAM {
public:
    ~FRAM() { }
    virtual size_t get_capacity() = 0;
    virtual bool read(int addr, void *data, size_t n_bytes) = 0;
    virtual bool write(int addr, const void *data, size_t n_bytes) = 0;
};

#endif
