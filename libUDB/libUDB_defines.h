// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2011 MatrixPilot Team
// See the AUTHORS.TXT file for a list of authors of MatrixPilot.
//
// MatrixPilot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixPilot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixPilot.  If not, see <http://www.gnu.org/licenses/>.

//	FBH	2021-04-18 changes for RTOM3


#ifndef UDB_DEFINES_H
#define UDB_DEFINES_H


#include "udbTypes.h"

#define NUM_POINTERS_IN(x)      (sizeof(x)>>1)

// Build for the specific board type
#define RED_BOARD               1   // red board with vertical LISY gyros (deprecated)
#define GREEN_BOARD             2   // green board with Analog Devices 75 degree/second gyros (deprecated)
#define UDB3_BOARD              3   // red board with daughter boards 500 degree/second Invensense gyros (deprecated)
#define RUSTYS_BOARD            4   // Red board with Rusty's IXZ-500_RAD2a patch board (deprecated)
#define UDB4_BOARD              5   // board with dsPIC33 and integrally mounted 500 degree/second Invensense gyros
#define CAN_INTERFACE           6
#define AUAV2_BOARD             7   // Nick Arsov's AUAV2 with dsPIC33 and MPU6000
#define UDB5_BOARD              8   // board with dsPIC33 and MPU6000
#define AUAV3_BOARD             9   // Nick Arsov's AUAV3 with dsPIC33EP and MPU6000
#define AUAV4_BOARD             10  // AUAV4 with PIC32MX

// Include the necessary files for the current board type
#if (BOARD_TYPE == UDB4_BOARD)
#include "ConfigUDB4.h"
#elif (BOARD_TYPE == UDB5_BOARD)
#include "ConfigUDB5.h"
#elif (BOARD_TYPE == AUAV3_BOARD)
#include "ConfigAUAV3.h"
#elif (BOARD_TYPE == CAN_INTERFACE)
#include "../CANInterface/ConfigCANInterface.h"
#else
#error "unsupported value for BOARD_TYPE"
#endif // BOARD_TYPE

// define the board rotations here.
// This include must go jsut after the board type has been declared
// Do not move this
// Orientation of the board
#define ORIENTATION_FORWARDS    0
#define ORIENTATION_BACKWARDS   1
#define ORIENTATION_INVERTED    2
#define ORIENTATION_FLIPPED     3
#define ORIENTATION_ROLLCW      4
#define ORIENTATION_ROLLCW180   5
#define ORIENTATION_YAWCW       6
#define ORIENTATION_YAWCCW      7

#include "boardRotation_defines.h"


// Clock configurations
#define CLOCK_CONFIG            3   // legacy definition for telemetry output


// Types
//#ifndef SIL_WINDOWS_INCS
typedef uint8_t boolean;
//#endif
#define true                    1
#define false                   0

struct ADchannel {
	int16_t input;  // raw input
	int16_t value;  // average of the sum of inputs between report outs
	int16_t offset; // baseline at power up 
	int32_t sum;    // used as an integrator
}; // variables for processing an AD channel


struct udb_flag_bits {
	uint16_t unused                 : 12;
	uint16_t a2d_read               : 1;
	uint16_t radio_on               : 1;
	uint16_t sonar_updated          : 1;
	uint16_t sonar_print_telemetry  : 1;
};


// LED states
#define LED_ON                    0
#define LED_OFF                   1

// relay and toner states
#define CLOSE_RELAY               1         // set output state - use 0 for LED (to pull LOW), 1 (apply 3.3 trigger) for FET
#define OPEN_RELAY                0         // use 1 for LED, 0 for FET

#define RELAY_CLOSED              0         // set input state - contacts pull input IN7/RD14 LOW to indicate closed
#define RELAY_OPEN                1

#define TONER_ON                  1         // set output state - use 0 for LED (to pull LOW), 1 (apply 3.3 trigger) for FET
#define TONER_OFF                 0         // 1 for LED, 0 for FET

// Channel numbers on the board, mapped to positions in the pulse width arrays.
#define CHANNEL_UNUSED          0   // udb_pwIn[0], udb_pwOut[0], etc. are not used, but allow lazy code everywhere else  :)
#define CHANNEL_1               1
#define CHANNEL_2               2
#define CHANNEL_3               3

//  FBH 2021-04-21
/*
#define CHANNEL_4               4
#define CHANNEL_5               5
#define CHANNEL_6               6
#define CHANNEL_7               7
#define CHANNEL_8               8
#define CHANNEL_9               9
#define CHANNEL_10              10
#define CHANNEL_11              11
#define CHANNEL_12              12
#define CHANNEL_13              13
#define CHANNEL_14              14
#define CHANNEL_15              15
#define CHANNEL_16              16
*/


// Constants
#define RMAX                    16384//0b0100000000000000       // 1.0 in 2.14 fractional format
#define GRAVITY                 ((int32_t)(5280.0/SCALEACCEL))  // gravity in AtoD/2 units

#define SERVOCENTER             3000
#define SERVORANGE              ((int16_t)(1000))
#define SERVOMAX                (SERVOCENTER + SERVORANGE)
#define SERVOMIN                (SERVOCENTER - SERVORANGE)

extern int16_t magMessage;
extern int16_t vref_adj;

#define NETWORK_INTERFACE_NONE                  0
#define NETWORK_INTERFACE_WIFI_MRF24WG          1
#define NETWORK_INTERFACE_ETHERNET_ENC624J600   2
#define NETWORK_INTERFACE_ETHERNET_ENC28J60     3

#endif // UDB_DEFINES_H
