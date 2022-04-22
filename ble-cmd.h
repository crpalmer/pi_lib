/*****************************************************************************
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of theex Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
******************************************************************************/
#ifndef _CMD_H_
#define _CMD_H_

#define BLE_CMD_PREFIX "AT+"

// Power-on return message
const char *Power_On_Return_Message_OFF = "CR00";
const char *Power_On_Return_Message_ON  = "CR01";

// Restore Factory Defaults
const char *actory_Reset                = "CW";

// Reset
const char *Reset                       = "CZ";

// Set baud rate
const int CMD_baud[] = { 9600, 19200, 38400, 57600, 115200, 256000, 512000, 230400, 460800, 10000000, 31250, 2400, 4800 };
const int n_CMD_baud = sizeof(CMD_baud) / sizeof(CMD_baud[0]);

// Query baud rate
const char *Baud_Rate_Query             = "QT";

// Chip low power Settings
const char *Not_Low_Power               = "CL00";
const char *Low_Power                   = "CL01";
// Chip low power Query
const char *Low_Power_Query             = "QL";

// Set the bluetooth name and address
const char *ame_BLE_Set                 = "BMBLE-Waveshare";
const char *ame_SPP_Set                 = "BDSPP-Waveshare";
const char *DD_SET                      = "BN112233445566";
// Example Query the name and address of bluetooth
const char *ame_BLE_Query               = "TM";
const char *ame_SPP_Query               = "TD";
const char *DD_Query                    = "TN";

// ON or OFF BLE
const char *LE_ON                       = "B401";
const char *LE_OFF                      = "B400";
// BLE Switch Query
const char *LE_Switch_Query             = "T4";

// ON or OFF SPP
const char *PP_ON                       = "B501";
const char *PP_OFF                      = "B500";
// SPP Switch Query
const char *PP_Switch_Query             = "T5";

// ERROR
const char *ERROR_1                     = "ER+1";
const char *ERROR_2                     = "ER+2";
const char *ERROR_3                     = "ER+3";
const char *ERROR_4                     = "ER+4";
const char *ERROR_5                     = "ER+5";
const char *ERROR_6                     = "ER+6";
const char *ERROR_7                     = "ER+7";
const char *ERROR_8                     = "ER+8";

#endif
