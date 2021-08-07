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

//	RTOM3 FBH Revisions
//	2021-03-04	Port dsPIC33FJ256GP710A-based board to dsPIC33FJ64GP206A-based board
//	2021-03-07	Delete some I/O defines; move/create related defines RTOM.h
//	2021-05-29  Changes for RTOM3

// used for the UDB5
// FBH - also used for the RTOM3; looks cumbersome to create ROMT3 only configuration
//       note that refs to MPU6000 are now refs to the ICM-20689 sensor

// ACCEL_RANGE must be either 4 or 8
// ACCEL_RANGE 2 will cause all sorts of trouble, do not use it
#define ACCEL_RANGE         4       // 4 g range

// note : it is possible to use other accelerometer ranges on the MPU6000
//#define SCALEGYRO           3.0016  // 500 degree/second range
//#define SCALEGYRO           2.9716  // 500 degree/second range
//#define SCALEGYRO           2.95  // 500 degree/second range
#define SCALEGYRO           2.935  // 500 degree/second range
//#define SCALEGYRO           1.47  // 500 degree/second range
//#define SCALEACCEL          1.29    // 4 g range
#define SCALEACCEL          1.27    // 4 g range measured by WJP on a few UDB5s

#define NUM_ANALOG_INPUTS   4

// A/D channels:
#define analogInput1BUFF    3
#define analogInput2BUFF    4
#define analogInput3BUFF    5
#define analogInput4BUFF    6
#define A_VOLT_BUFF         7       // V, pin label Bat Volt
#define A_AMPS_BUFF         8       // I, pin label CS Curr
#define A_RSSI_BUFF         9       // RS, pin label RSSI

#define A_VCC_BUFF          1
#define A_5V_BUFF           2

// MPU6000 configuration
// device is rotated 90 degrees clockwise from breakout board/AUAV3
// y -> x
// x -> -y

#define xrate_MPU_channel   4
#define yrate_MPU_channel   5
#define zrate_MPU_channel   6
#define temp_MPU_channel    3
#define xaccel_MPU_channel  0
#define yaccel_MPU_channel  1
#define zaccel_MPU_channel  2

#define XRATE_SIGN          +
#define YRATE_SIGN          -
#define ZRATE_SIGN          -
#define XACCEL_SIGN         -
#define YACCEL_SIGN         +
#define ZACCEL_SIGN         +

// Max inputs and outputs
#define MAX_INPUTS          8

// FBH  2021-04-21
#define MAX_OUTPUTS         3   // was 8

// SPI SS pin definitions
// FBH - change for RTOM3
//#define SPI1_SS             _LATB2
#define SPI2_SS             _LATG9

//#define SPI1_TRIS           _TRISB2
#define SPI2_TRIS           _TRISG9

//	FBH	2021-05-29  basic I/O for RTOM3 setup using 206A board
#define LED_BLUE            LATDbits.LATD5          // used to monitor/follow RELAY_POSITION (RD11)
#define LED_GREEN           LATFbits.LATF1
#define LED_RED             LATFbits.LATF0

#define TONER			   	LATBbits.LATB5			// Drives an FET for the beeper
#define RELAY               LATBbits.LATB4          // drives a FET for the relay

// input pin definitions
#define RELAY_POSITION             		PORTDbits.RD11

// pin locations of the hardware jumpers
#define JUMPER_1			(PORTBbits.RB3   == 0)
#define JUMPER_2			(PORTDbits.RD9   == 0)
#define JUMPER_3			(PORTDbits.RD10  == 0)
#define JUMPER_4			(PORTCbits.RC1  == 0)
