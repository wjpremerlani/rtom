// This file is part of the MatrixPilot RollPitchYaw demo.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2013 MatrixPilot Team
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
//  2021-07-23  Deleted RED LED (not on RTOM3) and added its tilt detect to GREEN LED for testing help
//  2021-07-28  Changed GRN LED off tilt angle from 30 to 45 degrees to facilitate test of contract assembled boards
//              Change logger output from Bill's stuff to RTOM3 user stuff
//  2021-08-07  Add fields to logger output (tilt and motion flags); adjusted/added mode and state names
//  2021-08-08  Changed ignition_en flag to ignition_dis

// main program for testing the IMU.


#include "../libDCM/libDCM.h"
#include "../libUDB/heartbeat.h"
#include "../libUDB/libUDB_internal.h"
#include "options.h"
#include "rtom.h"
#include "tiltLib.h"

// Used for serial debug output
#include <stdio.h>

char debug_buffer[512];
int db_index = 0;
void send_debug_line(void);

extern int16_t omega[] ;
extern int16_t omegacorrI[] ;
extern fractional theta[3] ;

struct relative2D roll_reference ;
int16_t roll_angle ;
int16_t rect_to_polar16(struct relative2D *xy);

#define RECORD_OFFSETS	( 0 ) // set to 1 in order to record accelerometer and gyro offsets in telemetry

int main(void)
{

// FBH - revise for RTOM3/206A
//	_TRISA2 = 1 ; // SCL is input pin with pullup
//	_TRISA3 = 1 ; // SDA is input pin with pullup
	_TRISG14 = 1 ; // for manual launch
	mcu_init();

	// Set up the libraries
	udb_init();
	dcm_init();
	udb_init_pwm();
	rtom_init();

	udb_serial_set_rate(SERIAL_BAUDRATE);

	LED_GREEN = LED_OFF;

	// Start it up!
	while (1)
	{
		udb_run();
	}

	return 0;
}

int16_t tenths = 0 ;
int16_t seconds = 0 ;
int16_t minutes = 0 ;

int16_t launched = 0 ;
int16_t launch_count = 0 ;

#define COS_30 (14189)
#define COS_45 (11585)          // need an angle to show that system is working after programming contracted board (w/o THT) and as a general tell-tale; 45 degrees will
                                // put it past what user normally does, so it will look like lit pilot normally - so change below from 30 to 45

// Called at HEARTBEAT_HZ
void dcm_heartbeat_callback(void) // was called dcm_servo_callback_prepare_outputs()
{
	{
        
// FBH - revised for RTOM3/206A        
		if ( _RG14 == 0 )                                   // ground test simulate launch by pulling RG14 low (ML test point)           
        {
			launched = 1 ;
		}
        
		if (( launched == 1 ) || ( rmat[7] > - COS_45 ))    // GREEN LED off if launched or tilt > 45 degrees; this will help evaluate newly assembled boards
		{
			LED_GREEN = LED_OFF ;
		}
		else
		{
			LED_GREEN = LED_ON ;
		}
        
#if ( HORIZONTAL_MOUNT == 1)
		if (( rmat[8]> COS_30 ) || (launched == 1))
		{
			LED_GREEN = LED_OFF ;
		}
		else
		{
			LED_GREEN = LED_ON ;
		}

#endif
		rtom();
	}

//Serial output at 10Hz
	if (udb_heartbeat_counter % 4 == 0)
	{
		if (dcm_flags._.calib_finished)
		{
			send_debug_line();
		}
	}
}

int16_t accelOn ;
int16_t line_number = 1 ;

// Prepare a line of serial output and start it sending
extern double firmware ;
extern int ignition_dis ;
extern int max_tilt ;
double flt_time ;
double elapsed_time ;
extern int state ;
extern int option_mode ;
extern int tilt_flag ;
extern int energy_flag ;

const char *state_name[]= {"",
"Pre_launch",
"Motion Off",
"Reentry",
"Motion On",
"Lockout"} ;

const char *mode_name[]= {"",
"Abort",
"Abort-Motion",
"Reentry",
"Reentry-Motion"} ;


//	Send various output to the OpenLog card
void send_debug_line( void )
{
	if (( launched == 1 ) && (flt_time < 120.1 ))
	{
	db_index = 0 ;
	sprintf( debug_buffer ,

//  Standard output:
				  "FW: %3.2f, VRR: %i , Flt_Time: %5.1f\r\n"
				  "Mode: %s , State: %s , Launched: %i\r\n"
                  "Excess Tilt: %i , Excess Motion: %i\r\n"
				  "Crit_Angle: %i , Cur_Angle: %3.1f , Ign_Disabled: %i\r\n"
				  "   \r\n" ,

		firmware , omegagyro[1]/16 , flt_time ,
		mode_name[option_mode] , state_name[state] , launched ,
        tilt_flag , energy_flag ,    
		max_tilt , (double)tilt_angle() , ignition_dis ) ;

	udb_serial_start_sending_data() ;

	if (launched == 1)
	{
		flt_time = flt_time + 0.1 ;
	}
	else
	{
		flt_time = 0 ;
	}

//	elapsed_time = elapsed_time + 0.1 ;
	
	return ;
    }
}


// Bill's output
/*
void send_debug_line(void)
{
	db_index = 0;
	if( RECORD_OFFSETS == 1 )
	{
		sprintf( debug_buffer , "%i, %i, %i, %i, %i, %i\r\n" , 
			udb_xaccel.value , udb_yaccel.value , udb_zaccel.value , udb_xrate.value , udb_yrate.value , udb_zrate.value ) ; 
	}
	else switch ( line_number )
	{	
		case 5 :
		{
			{
				sprintf( debug_buffer , "gyroXoffset, gyroYoffset, gyroZoffset, percentCPUload\r\n" ) ;
			}
			line_number ++ ;
			break ;
		}	
		case 4 :
		{
			sprintf( debug_buffer , "time, accelOn, launchCount, launched, rollAngle,  vertX, vertY, vertZ, accX, accY, accZ, gyroX, gyroY, gyroZ, " ) ;
			line_number ++ ;
			break ;
		}
		case 3 :
		{
			line_number ++ ;
			return ;
		}
		case 2 :
		{
			line_number ++ ;
			return ;
		}
		case 1 :
		{
			line_number ++ ;
			return ;
		}
		case 6 :
	{
		roll_reference.x = rmat[0];
		roll_reference.y = rmat[3];
		roll_angle = rect_to_polar16(&roll_reference) ;
			sprintf(debug_buffer, "%i:%2.2i.%.1i,%i,%i,%i,%.2f,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i\r\n",
			minutes, seconds , tenths , accelOn, launch_count, launched , ((double)roll_angle)/(182.0) , 
			rmat[6], rmat[7], rmat[8] ,
			-( udb_xaccel.value)/2 + ( udb_xaccel.offset ) / 2 , 
			( udb_yaccel.value)/2 - ( udb_yaccel.offset ) / 2 ,
			( udb_zaccel.value)/2 - ( udb_zaccel.offset ) / 2 ,
			omegaAccum[0] ,
			omegaAccum[1] ,
			omegaAccum[2] ,
			omegacorrI[0] ,
			omegacorrI[1] ,
			omegacorrI[2] ,
			(uint16_t) udb_cpu_load() );
			tenths ++ ;
			if ( tenths == 10 )
			{
				tenths = 0 ;
				seconds++ ;
				if ( seconds == 60 )
				{
					seconds = 0 ;
					minutes++ ;
				}
			}
			break ;
		}
	}
	udb_serial_start_sending_data();
}
*/


// Return one character at a time, as requested.
// Requests will stop after we send back a -1 end-of-data marker.
int16_t udb_serial_callback_get_byte_to_send(void)
{
	uint8_t c = debug_buffer[db_index++];

	if (c == 0) return -1;
	return c;
}

// Don't respond to serial input
void udb_serial_callback_received_byte(uint8_t rxchar)
{
	// Do nothing
}

