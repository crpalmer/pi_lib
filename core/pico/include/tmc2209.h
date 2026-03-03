#ifndef __TMC2209_H__
#define __TMC2209_H__

#include "uart.h"

class TMC2209 {
public:
     TMC2209(UART_Tx *tx, uint8_t address) : tx(tx), address(address) {
	set_defaults();
     }

     bool set_microstepping(int microsteps, bool interpolate = true);
     bool set_rms_current(int mA);

private:
     UART_Tx *tx;
     uint8_t  address;

     void set_defaults();
     bool set_register(uint8_t address, uint32_t value);

     uint32_t gconf;
     uint32_t chopconf;
     uint32_t iholdirun;
};

#endif
