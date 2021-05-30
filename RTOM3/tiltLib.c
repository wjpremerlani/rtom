#include "tiltLib.h"
#include "options.h"
#include "../libDCM/libDCM.h"
#include "../libUDB/heartbeat.h"
#include "../libUDB/libUDB_internal.h"
#include "../libDCM/mathlibNav.h"

//#define DEBUG_TILTLIB

#ifdef DEBUG_TILTLIB
#undef HORIZONTAL_MOUNT
#define HORIZONTAL_MOUNT ( 0 )
#endif

extern fractional omega[] ;

#if ( GYRO_RANGE == 500 )
#define GYRO_FACTOR ( 65.5 ) // UDB and RTOM sensitivity
#elif ( GYRO_RANGE == 1000 )
#define GYRO_FACTOR ( 32.8 ) // UDB and RTOM sensitivity
#else
#error GYRO_RANGE not specified as 500 or 1000
#endif // GYRO_RANGE


int16_t sine_max_tilt = 5460 ; // default max tilt is 30 degrees until init_tilt_parameters is called

int16_t max_energy = (int16_t) ((GYRO_FACTOR/2)*10.0) ; //default max energy is 10 degrees per second

uint16_t look_back_time = 400 ; // default lookback time is 10 seconds 
uint16_t look_back_count = 400 ;

void init_tilt_parameters ( float max_tilt , float max_tilt_rate , float look_back_time_float)
{
	sine_max_tilt = sine16( (uint16_t) 182.04*max_tilt) ;
	max_energy = (int16_t) ((GYRO_FACTOR/2)*max_tilt_rate);
	look_back_time = (uint16_t) 40.0*look_back_time_float ;
	look_back_count = look_back_time ;
}

int16_t tilt_ok()
{
	int32_t tilt_margin ;
	int32_t energy_margin ;
	tilt_margin = __builtin_mulss( sine_max_tilt , sine_max_tilt ) ;
	energy_margin = __builtin_mulss( max_energy , max_energy ) ;
	
#if ( HORIZONTAL_MOUNT == 1)
	tilt_margin -= __builtin_mulss( rmat[6] , rmat[6] ) ;
	tilt_margin -= __builtin_mulss( rmat[7] , rmat[7] ) ;
	energy_margin -= __builtin_mulss( omega[0] , omega[0] ) ;
	energy_margin -= __builtin_mulss( omega[1] , omega[1] ) ;
	
	if (( rmat[8]> 0 ) && ( tilt_margin > 0 ) && ( energy_margin > 0 ))
	{
		if ( look_back_count > 0 )
		{
			look_back_count -- ;
			return 0 ;
		}
		else
		{
			return 1 ;
		}
	}
	else
	{
		look_back_count = look_back_time ;
		return 0 ;
	}
#else
	tilt_margin -= __builtin_mulss( rmat[6] , rmat[6] ) ;
	tilt_margin -= __builtin_mulss( rmat[8] , rmat[8] ) ;
	energy_margin -= __builtin_mulss( omega[0] , omega[0] ) ;
	energy_margin -= __builtin_mulss( omega[2] , omega[2] ) ;
	if (( rmat[7]< 0 ) && ( tilt_margin > 0 ) && ( energy_margin > 0 ))
{
		if ( look_back_count > 0 )
		{
			look_back_count -- ;
			return 0 ;
		}
		else
		{
			return 1 ;
		}
	}
	else
	{
		look_back_count = look_back_time ;
		return 0 ;
	}
#endif
	
}