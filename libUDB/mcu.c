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

// 2021-03-04	Port dsPIC33FJ256GP710A-based board to dsPIC33FJ64GP206A-based board
//         07   Adjust inputs, notes and add relay and buzzer configuration 

#include "libUDB_internal.h"
#include "oscillator.h"
#include "interrupt.h"
#include "uart.h"
#include <stdio.h>

#if (BOARD_TYPE == UDB4_BOARD || BOARD_TYPE == UDB5_BOARD)
#include <p33Fxxxx.h>
#ifdef __XC16__
#pragma config FNOSC = PRIPLL
#pragma config FCKSM = CSDCMD
#pragma config OSCIOFNC = OFF
#pragma config POSCMD = XT
#pragma config FWDTEN = OFF
#pragma config WINDIS = OFF
#pragma config GSS = OFF
#pragma config GWRP = OFF
#pragma config FPWRT = PWR1
#pragma config JTAGEN = OFF
#pragma config ICS = PGD2

#else // Not __XC16__
_FOSCSEL(FNOSC_PRIPLL); // pri plus PLL (primary osc  w/ PLL)
_FOSC(FCKSM_CSDCMD &
      OSCIOFNC_OFF &
      POSCMD_XT);
// Clock switching on startup is enabled, starts with fast RC.
// Clock switching after startup is disabled.
// Fail-Safe Clock Monitor is disabled.
// OSC2 pin has clock out function.
// Primary Oscillator XT mode.
_FWDT(FWDTEN_OFF &
      WINDIS_OFF);
_FGS(GSS_OFF &
     GCP_OFF &
     GWRP_OFF);
_FPOR(FPWRT_PWR1);
_FICD(JTAGEN_OFF &
      ICS_PGD2);
#endif // __XC16__

#endif // BOARD_TYPE

#define INPUT_PIN  1
#define OUTPUT_PIN 0

int16_t defaultCorcon = 0;

volatile uint16_t trap_flags __attribute__ ((persistent, near));
volatile uint32_t trap_source __attribute__ ((persistent, near));
volatile uint16_t osc_fail_count __attribute__ ((persistent, near));

uint16_t get_reset_flags(void)
{
	uint16_t oldRCON = RCON;
	RCON = 0;
	trap_flags = 0;
	trap_source = 0;
	osc_fail_count = 0;
	return oldRCON;
}

void configureDigitalIO(void) // UDB4 and UDB5 boards FBH - and RTOM3
{

// FBH - revise for RTOM3/206A
//	_TRISD8 = 1;   FBH - RD8 used for GPS, which will be eliminated later

// FBH - remove    
//	_TRISD9 = _TRISD10 = _TRISD11 = _TRISD12 = _TRISD13 = _TRISD14 = _TRISD8;

// FBH - seems to be over-ridden by other assignments - let's see,
//	TRISF = 0b1111111111101100;

// FBH - relay and buzzer
   	_LATB4 = 0; _LATB5 = 0;    
   	_TRISB4 = 0; _TRISB5 = 0;
          
}

void init_leds(void)
{
#if (BOARD_TYPE == UDB4_BOARD || BOARD_TYPE == UDB5_BOARD)
    
// FBH - revise for RTOM3/206A    
//	_LATE1 = LED_OFF; _LATE2 = LED_OFF; _LATE3 = LED_OFF; _LATE4 = LED_OFF;
//	_TRISE1 = 0; _TRISE2 = 0; _TRISE3 = 0; _TRISE4 = 0;
   	_LATF0 = LED_OFF; _LATF1 = LED_OFF;
	_TRISF0 = 0; _TRISF1 = 0;

#else
#error Invalid BOARD_TYPE
#endif // BOARD_TYPE
}

void mcu_init(void)
{
	defaultCorcon = CORCON;

	if (_SWR == 0)
	{
		// if there was not a software reset (trap error) clear the trap data
		trap_flags = 0;
		trap_source = 0;
		osc_fail_count = 0;
	}

#if (BOARD_TYPE == UDB4_BOARD || BOARD_TYPE == UDB5_BOARD)
#if (MIPS == 16)
	CLKDIVbits.PLLPRE = 0;  // PLL prescaler: N1 = 2 (default)
	CLKDIVbits.PLLPOST = 1; // PLL postscaler: N2 = 4 (default)
	PLLFBDbits.PLLDIV = 30; // FOSC = 32 MHz (XTAL=8MHz, N1=2, N2=4, M = 32)
#elif (MIPS == 32)
#warning 32 MIPS selected
	CLKDIVbits.PLLPRE = 0;  // PLL prescaler: N1 = 2 (default)
	CLKDIVbits.PLLPOST = 0; // PLL postscaler: N2 = 2
	PLLFBDbits.PLLDIV = 30; // FOSC = 64 MHz (XTAL=8MHz, N1=2, N2=2, M = 32)
#elif (MIPS == 40)
#warning 40 MIPS selected
	CLKDIVbits.PLLPRE = 0;  // PLL prescaler: N1 = 2 (default)
	CLKDIVbits.PLLPOST = 0; // PLL postscaler: N2 = 2
	PLLFBDbits.PLLDIV = 38; // FOSC = 80 MHz (XTAL=8MHz, N1=2, N2=2, M = 40)
#else
#error "invalid MIPS Configuration"
#endif // MIPS
#endif // BOARD_TYPE
	configureDigitalIO();
	init_leds();

}
