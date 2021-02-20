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


// main program for testing the IMU.


#include "../libDCM/libDCM.h"
#include "../libUDB/heartbeat.h"
#include "../libUDB/libUDB_internal.h"

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
	_TRISA2 = 1 ; // SCL is input pin for enabling yaw/pitch control
	_TRISA3 = 1 ; // SDA is input pin for enabling roll control
	_TRISD15 = 0 ; // repurpose PWM8 input as an output
	_LATD15 = 0 ;
	mcu_init();

	// Set up the libraries
	udb_init();
	dcm_init();
	udb_init_pwm();

	udb_serial_set_rate(SERIAL_BAUDRATE);

	LED_GREEN = LED_OFF;

	// Start it up!
	while (1)
	{
		udb_run();
	}

	return 0;
}


// Called every 1/40 second at high priority
void udb_heartbeat_40hz_callback(void)
{
	static int count = 0;

	if (!dcm_flags._.calib_finished)
	{
		// If still calibrating, blink RED
		if (++count > 20)
		{
			count = 0;
			udb_led_toggle(LED_RED);
		}
	}
	else
	{
		// No longer calibrating: solid RED and send debug output
		LED_RED = LED_ON;
	}
}

int16_t tenths = 0 ;
int16_t seconds = 0 ;
int16_t minutes = 0 ;

int16_t launched = 0 ;
int16_t launch_count = 0 ;

#define VERTICAL_MOUNT  1 
#define HORIZONTAL_MOUNT  2 

// Called at HEARTBEAT_HZ, before sending servo pulses
void dcm_heartbeat_callback(void) // was called dcm_servo_callback_prepare_outputs()
{
	{
		if ( GROUND_TEST == 1 )
		{
			if ( ( _RA2 == 0 ) && ( _RA3 == 0 ) ) // ground test simulate launch by enabling both control modes
			{
				launched = 1 ;
			}
		}
	}

//  // Serial output at 10Hz
	if (udb_heartbeat_counter % 4 == 0)
	{
		if (dcm_flags._.calib_finished)
		{
			send_debug_line();
		}
	}
}

int16_t accelOn ;

#if ( GYRO_RANGE == 500 )
#define GYRO_FACTOR ( 65 ) // UDB5 sensitivity
#elif ( GYRO_RANGE == 1000 )
#define GYRO_FACTOR ( 32 ) // UDB5 sensitivity
#else
#error GYRO_RANGE not specified as 500 or 1000
#endif // GYRO_RANGE

int16_t line_number = 1 ;
// Prepare a line of serial output and start it sending
void send_debug_line(void)
{
	db_index = 0;
	if( RECORD_OFFSETS == 1 )
	{
//		int16_t gravity2x = (int16_t) 2*GRAVITY ;
//		sprintf( debug_buffer , "%i, %i, %i, %i, %i, %i, %i\r\n" , 
//			gravity2x, udb_xaccel.value , udb_yaccel.value , udb_zaccel.value , udb_xrate.value , udb_yrate.value , udb_zrate.value ) ; 
		sprintf( debug_buffer , "%i, %i, %i, %i, %i, %i\r\n" , 
			udb_xaccel.value , udb_yaccel.value , udb_zaccel.value , udb_xrate.value , udb_yrate.value , udb_zrate.value ) ; 
	}
	else switch ( line_number )
	{
		
		case 5 :
		{
			if ( GROUND_TEST == 1)
			{
				sprintf( debug_buffer , "gyroXoffset, gyroYoffset, gyroZoffset,yawFbVert, pitchFbVert, rollFbVert, yawFbHoriz, pitchFbHoriz, rollFbHoriz\r\n" ) ;
			}
			else
			{
				sprintf( debug_buffer , "yawFbVert, pitchFbVert, rollFbVert, yawFbHoriz, pitchFbHoriz, rollFbHoriz, out1, out2, out3, out4, out5, out6, out7, out8\r\n" ) ;
			}
			line_number ++ ;
			break ;
		}
		
		case 4 :
		{
			sprintf( debug_buffer , "time, cntlModeYwPtch, cntlModeRoll, accelOn, launchCount, launched, tilt_count, apogee , rollAngle, rollDeviation, vertX, vertY, vertZ, accX, accY, accZ, gyroX, gyroY, gyroZ, " ) ;
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
//		sprintf( debug_buffer , "JJBrd1, rev13, 5/10/2015\r\nTiltMultiplier: %i, RollMultiplier: %i\r\n" , BOARD, REVISION, DATE, (int16_t) TILT_GAIN , (int16_t) SPIN_GAIN ) ;
//		sprintf( debug_buffer , "%s, %s, %s\r\nTiltMultiplier: %i, RollMultiplier: %i\r\nSensorOffsets, Accel: , %i, %i, %i, Gyro: , %i, %i, %i\r\n" , 
			sprintf( debug_buffer , "Max Roll Angle = %i deg.\r\nOffsets, Accel: , %i, %i, %i, Gyro: , %i, %i, %i\r\n" , 
			0 ,
			udb_xaccel.offset , udb_yaccel.offset , udb_zaccel.offset ,
			udb_xrate.offset , udb_yrate.offset , udb_zrate.offset
			 	) ;
			line_number ++ ;
			break ;
		}
		case 1 :
		{
		sprintf( debug_buffer , "%i, %i, %i\r\nGyro range %i DPS, calibration %i\r\nTiltAngle %i deg, TiltRate %i deg/s, %i usecs.\r\nSpin %i deg/sec %i usecs.\r\n" ,
			0, 0, 0, GYRO_RANGE , 0 ,
			0 , 0 ,(int16_t) 0 , (int16_t) 0 , (int16_t) 0		
			 	) ;
		line_number ++ ;
		break ;
		}
		case 6 :
	{
		roll_reference.x = rmat[0];
		roll_reference.y = rmat[3];
		roll_angle = rect_to_polar16(&roll_reference) ;
			sprintf(debug_buffer, "%i:%2.2i.%.1i,%i,%i,%i,%i,%i,%i,%i,%.2f,%i,%i,%i,%i,%i,%i,%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%i,%i,%i,%i,%i\r\n",
			minutes, seconds , tenths ,  0, 0 , accelOn, launch_count, launched , 0 , 0, ((double)roll_angle)/(182.0) , 
			0,
			rmat[6], rmat[7], rmat[8] ,
			-( udb_xaccel.value)/2 + ( udb_xaccel.offset ) / 2 , 
			( udb_yaccel.value)/2 - ( udb_yaccel.offset ) / 2 ,
			( udb_zaccel.value)/2 - ( udb_zaccel.offset ) / 2 ,
			((double)(  omegaAccum[0])) / ((double)( GYRO_FACTOR/2 )) ,
			((double)(  omegaAccum[1])) / ((double)( GYRO_FACTOR/2 )) ,
			((double)(  omegaAccum[2])) / ((double)( GYRO_FACTOR/2 )) ,
			((double)( omegacorrI[0])) / ((double)( GYRO_FACTOR/2 )) ,
			((double)( omegacorrI[1])) / ((double)( GYRO_FACTOR/2 )) ,
			((double)( omegacorrI[2])) / ((double)( GYRO_FACTOR/2 )) ,
			0 ,
			0 ,
			0 ,
			0 ,
			0 ,
			0 ) ;

//			(uint16_t) udb_cpu_load() );
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

