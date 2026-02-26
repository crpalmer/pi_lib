#include "pi.h"
#include "consoles.h"
#include "tmc2209.h"

typedef struct {
    const char *name;
    int n_bits;
} bit_mapping_t;

typedef struct {
    const char    *name;
    uint8_t        reg;
    const bit_mapping_t *m;
} bit_mappings_t;

static const int REG_GCONF    = 0x00;
static const int REG_CHOPCONF = 0x6C;

#define DEBUG 1

static const bit_mapping_t bit_mapping_gconf[] = {
    { "I_scale_analog", 1 },
    { "internal_Rsense", 1 },
    { "en_SpreadCycle", 1},
    { "shaft", 1 },
    { "index_otpw", 1},
    { "index_step", 1},
    { "pdn_disable", 1},
    { "mstep_reg_select", 1},
    { "multistep_filt", 1 },
    { "test_mode", 1},
    { "reserved", 22},
    { NULL, 0}
};

static const bit_mapping_t bit_mapping_chopconf[] = {
    { "toff",      4 },
    { "hstrt",     3 },
    { "hend",      4 },
    { "reserved",  4 },
    { "tbl",       2 },
    { "vsense",    1 },
    { "reserved2", 6 },
    { "mres",      4 },
    { "intpol",    1 },
    { "dedge",     1 },
    { "diss2g",    1 },
    { "diss2vs",   1 },
    { NULL, 0 }
};

/***************************************************/

static const bit_mappings_t bit_mappings[] = {
    { "chopconf",	REG_CHOPCONF,	bit_mapping_chopconf},
    { "gconf",		REG_GCONF,	bit_mapping_gconf},
    { NULL, 0 }
};

static void bit_mapping_set(const bit_mapping_t *m, uint32_t *reg, const char *name, uint32_t value) {
    int b0 = 0;

    for (; m->name && b0 < 32; m++) {
	if (strcmp(m->name, name) == 0) {
	    uint32_t mask = (1 << m->n_bits)-1;
	    value &= mask;
	    *reg = (*reg & ~(mask << b0)) | (value << b0);
	    return;
	}

	b0 += m->n_bits;
    }

    if (b0 > 32 || m->name) {
	consoles_fatal_printf("\n**** ERROR : bit_mapping_t structure contains more than 32 bits!! *****\n\n");
    } else if (b0 < 32) {
	consoles_fatal_printf("\n**** ERROR : bit_mapping_t structure contains less than 32 bits *****\n\n");
    } else {
	consoles_fatal_printf("\n**** ERROR : Couldn't find bit mapping for \"%s\" *****\n", name);
    }
}

static void bit_mapping_dump(const bit_mapping_t *m, uint32_t reg) {
    int b0 = 0;

    printf("%08lx =>\n", reg);
    for (; m->name && b0 < 32; m++) {
	uint32_t v = (reg >> b0) & ((1 << m->n_bits)-1);
	if (v) printf("  %s: 0x%lx\n", m->name, (reg >> b0) & ((1 << m->n_bits)-1));
	b0 += m->n_bits;
    }
}

static void bit_mappings_dump(const char *msg, uint8_t reg, uint32_t value) {
#if DEBUG
    for (int i = 0; bit_mappings[i].name; i++) {
	if (bit_mappings[i].reg == reg) {
	    printf("%s: %s (reg 0x%02x): ", msg, bit_mappings[i].name, reg);
	    bit_mapping_dump(bit_mappings[i].m, value);
	    return;
	}
    }
#endif
}

void TMC2209::set_defaults() {
    chopconf = 0x10000053;
    set_register(REG_CHOPCONF, chopconf);
    gconf = 0;
    bit_mapping_set(bit_mapping_gconf, &gconf, "I_scale_analog", 1);
    bit_mapping_set(bit_mapping_gconf, &gconf, "pdn_disable", 1);
    bit_mapping_set(bit_mapping_gconf, &gconf, "mstep_reg_select", 1);
    bit_mapping_set(bit_mapping_gconf, &gconf, "multistep_filt", 1);
bit_mapping_set(bit_mapping_gconf, &gconf, "shaft", 1);
    set_register(REG_GCONF, gconf);
}

bool TMC2209::set_register(uint8_t reg, uint32_t data) {
    uint8_t msg[8];
    msg[0] = 5;			// sync + reserved = 0
    msg[1] = address;
    msg[2] = (reg & 0x7f) | 0x80; /* reg + rw bit */
    msg[3] = (data>>24) & 0xff;
    msg[4] = (data>>16) & 0xff;
    msg[5] = (data>>8) & 0xff;
    msg[6] = (data>>0) & 0xff;

    uint8_t crc = 0;
    for (unsigned i = 0; i < sizeof(msg)-1; i++) {
	uint8_t byte = msg[i];
	for (int j = 0; j < 8; j++) {
	    if ((crc >> 7) ^ (byte & 0x01)) {
		crc = (crc << 1) ^ 0x07;
	    } else {
		crc = (crc << 1);
	    }
	    byte >>= 1;
	}
    }

    msg[7] = crc;

#if DEBUG
    printf("send to tmc: %02x %02x %02x %02x%02x%02x%02x crc=%02x\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6], msg[7]);
#endif
    bit_mappings_dump(" sending", reg, data);

    tx->write(msg, sizeof(msg));
    return true;
}

bool TMC2209::set_microstepping(int steps_per_mm, bool interpolate) {
    uint32_t mres;

    switch (steps_per_mm) {
    case 256: mres = 0; break;
    case 128: mres = 1; break;
    case 64:  mres = 2; break;
    case 32:  mres = 3; break;
    case 16:  mres = 4; break;
    case 8:   mres = 5; break;
    case 4:   mres = 6; break;
    case 2:   mres = 7; break;
    case 1:   mres = 8; break;
    default: return false;
    }

    bit_mapping_set(bit_mapping_chopconf, &chopconf, "mres", mres);
    bit_mapping_set(bit_mapping_chopconf, &chopconf, "intpol", interpolate);
    return set_register(REG_CHOPCONF, chopconf);
}
