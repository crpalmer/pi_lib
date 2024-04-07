#ifndef __ADS1115_H__
#define __ADS1115_H__

#include "adc.h"
#include "i2c.h"

typedef enum {
    ADS1115_2_048V,
    ADS1115_1_024V,
    ADS1115_0_512V,
    ADS1115_0_256V
} ads1115_v_t;

typedef enum {
    ADS1115_8SPS = 0,
    ADS1115_16SPS,
    ADS1115_32SPS,
    ADS1115_64SPS,
    ADS1115_128SPS,
    ADS1115_250SPS,
    ADS1115_475SPS,
    ADS1115_860SPS
} ads1115_sps_t;

class ADS1115 : public ADC {
public:
    ADS1115(int bus = 0, int addr = 0x48) {
	i2c = i2c_open(bus, addr);
    }

    uint16_t read(int reg) {
	write_config(config_start_conversion(reg));
	while ((read_config() & 0x8000) == 0) {}
	return read_result();
    }

    double to_voltage(uint16_t r) {
	double sign = 1;
	if ((r & 0x8000) != 0) {
	    sign = -1;
	    r = (r ^ 0xffff) + 1;
	}
	return sign * max_volts() * r / 0x8000;
    }

    void set_max_volts(ads1115_v_t max_volts) {
	v = max_volts;
    }

    void set_samples_per_sec(ads1115_sps_t new_sps) {
	sps = new_sps;
    }

private:
    int i2c;
    ads1115_v_t v = ADS1115_2_048V;
    ads1115_sps_t sps = ADS1115_128SPS;

    uint16_t config_start_conversion(uint8_t reg) {
	return 00000    // Disable comparator
               | (sps << 5)   // samples / sec
	       | (1 << 8)   // single shot mode
	       | (max_volts_to_config_bits() << 9)   // voltage range
	       | ((4 + reg) << 12)
	       | (1 << 15)  // Start a conversion
	;
     }

     void write_config(uint16_t config) {
	uint8_t data[2] = { byte_of(config, 1), byte_of(config, 0) };
	if (i2c_write(i2c, 1, data, 2) < 0) {
	    perror("ads1115 write config");
	}
     }

     uint16_t read_config() {
	uint8_t data[2];
	if (i2c_read(i2c, 1, data, 2) < 0) {
	    perror("ads1115 read config");
	    return 0;
	}
	return data[0] << 8 | data[1];
     }

     uint16_t read_result() {
	uint8_t data[2];
	if (i2c_read(i2c, 0, data, 2) < 0) {
	    perror("ads1115 read result");
	    return 0;
	}
	return data[0] << 8 | data[1];
     }

     uint8_t byte_of(uint16_t pair, int byte) {
	return static_cast<uint8_t> ((pair >> (byte*8)) & 0xff);
     }

     double max_volts() {
	switch(v) {
	case ADS1115_2_048V: return 2.048;
	case ADS1115_1_024V: return 1.024;
	case ADS1115_0_512V: return 0.512;
	case ADS1115_0_256V: return 0.256;
	default: assert(0); return 0;
	}
     }

     uint8_t max_volts_to_config_bits() {
	switch(v) {
	case ADS1115_2_048V: return 2;
	case ADS1115_1_024V: return 3;
	case ADS1115_0_512V: return 4;
	case ADS1115_0_256V: return 5;
	default: assert(0); return 0;
	}
     }
	
};

#endif
