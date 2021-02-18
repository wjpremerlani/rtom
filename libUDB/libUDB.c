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


#include "libUDB_internal.h"
#include "oscillator.h"
#include "interrupt.h"

#if (USE_I2C1_DRIVER == 1)
#include "I2C.h"
#endif

union udb_fbts_byte udb_flags;

void udb_init(void)
{
	udb_flags.B = 0;

#if (USE_I2C1_DRIVER == 1)
	I2C1_Init();
#endif
	udb_init_clock();
#if (CONSOLE_UART != 2)
	udb_init_USART();
#endif

#if (BOARD_TYPE == UDB5_BOARD || BOARD_TYPE == AUAV3_BOARD)
	MPU6000_init16();
#endif

	SRbits.IPL = 0; // turn on all interrupt priorities
}

void udb_run(void)
{
#if (USE_MCU_IDLE == 1)
	Idle();
#else
	// pause cpu counting timer while not in an ISR
	indicate_loading_main;
#endif
}

void udb_a2d_record_offsets(void)
{

#ifdef CUSTOM_OFFSETS
	// offsets have been measured manually and entered into the options.h file
	udb_xaccel.offset = XACCEL_OFFSET ;
	udb_yaccel.offset = YACCEL_OFFSET ;
	udb_zaccel.offset = ZACCEL_OFFSET ;
	udb_xrate.offset = XRATE_OFFSET ;
	udb_yrate.offset = YRATE_OFFSET ;
	udb_zrate.offset = ZRATE_OFFSET ;
#else
	// almost ready to turn the control on, save the input offsets
	UDB_XACCEL.offset = UDB_XACCEL.value;
	udb_xrate.offset  = udb_xrate.value;
	UDB_YACCEL.offset = UDB_YACCEL.value;
	udb_yrate.offset  = udb_yrate.value;
	UDB_ZACCEL.offset = UDB_ZACCEL.value + (Z_GRAVITY_SIGN ((int16_t)(2*GRAVITY))); // same direction
	udb_zrate.offset  = udb_zrate.value;
#endif	// CUSTOM_OFFSETS
#ifdef VREF
	udb_vref.offset   = udb_vref.value;
#endif
}

// saturation logic to maintain pulse width within bounds
int16_t udb_servo_pulsesat(int32_t pw)
{
	if (pw > SERVOMAX) pw = SERVOMAX;
	if (pw < SERVOMIN) pw = SERVOMIN;
	return (int16_t)pw;
}
