#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pifacedigital.h"
#include "mcp23s17.h"
#include "piface.h"

struct pifaceS {
   unsigned hwaddr;
   bool     interrupts_enabled;
};

piface_t *
piface_new(void)
{
    return piface_new_id(0);
}

piface_t *
piface_new_id(unsigned hwaddr)
{
    piface_t *p = malloc(sizeof(*p));

    p->hwaddr = hwaddr;

    pifacedigital_open(hwaddr);
    p->interrupts_enabled = (pifacedigital_enable_interrupts() == 0);
    if (! p->interrupts_enabled == 0) {
        fprintf(stderr, "%s: Could not enable interrupts.\n", __func__);
    }

    return p;
}

void
piface_set(piface_t *p, unsigned pin, bool value)
{
    assert(p);
    pifacedigital_write_bit(value, pin, OUTPUT, p->hwaddr);
}

bool
piface_get(piface_t *p, unsigned pin)
{
    return pifacedigital_read_bit(pin, INPUT, p->hwaddr) == 0;
}

unsigned
piface_get_all(piface_t *p)
{
    unsigned raw = pifacedigital_read_reg(INPUT, p->hwaddr);
    return raw ^ 0xf;
}

void
piface_wait_for_input(piface_t *p)
{
    assert(p);
    assert(p->interrupts_enabled);

    pifacedigital_wait_for_input(-1, p->hwaddr);
}

void
piface_destroy(piface_t *p)
{
    assert(p);
    free(p);
}
