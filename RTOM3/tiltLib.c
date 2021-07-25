#include "tiltLib.h"
#include "math.h"
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


int16_t too_much_tilt(int16_t max_tilt )
// max_tilt is maximum allowable tilt in integer degrees
// returns 1 if max_tilt is exceeded, 0 otherwise
{
	int16_t sine_max_tilt = sine16(182*max_tilt);
	int32_t tilt_margin ;
	tilt_margin = __builtin_mulss( sine_max_tilt , sine_max_tilt ) ;
#if ( HORIZONTAL_MOUNT == 1)
	tilt_margin -= __builtin_mulss( rmat[6] , rmat[6] ) ;
	tilt_margin -= __builtin_mulss( rmat[7] , rmat[7] ) ;
	if (( rmat[8]> 0 ) && ( tilt_margin > 0 ))
	{
		return 0 ;
	}
	else
	{
		return 1 ;
	}	
#else
	tilt_margin -= __builtin_mulss( rmat[6] , rmat[6] ) ;
	tilt_margin -= __builtin_mulss( rmat[8] , rmat[8] ) ;
	if (( rmat[7]< 0 ) && ( tilt_margin > 0 ) )
	{
		return 0 ;
	}
	else
	{
		return 1 ;
	}
#endif	
}

int16_t too_much_energy(int16_t max_tilt_rate )
// max_tilt_rate is maximum allowable energy in integer degrees per second
// returns 1 if maximum energy is is exceeded, 0 otherwise
{
	uint16_t max_energy = (int16_t) ((GYRO_FACTOR/2)*max_tilt_rate);
	int32_t energy_margin = __builtin_mulss( max_energy , max_energy ) ;
#if ( HORIZONTAL_MOUNT == 1)
	energy_margin -= __builtin_mulss( omega[0] , omega[0] ) ;
	energy_margin -= __builtin_mulss( omega[1] , omega[1] ) ;
	if (( rmat[8]> 0 ) && ( energy_margin > 0 ))
	{	
		return 0 ;
	}
	else
	{
		return 1 ;
	}
#else
	energy_margin -= __builtin_mulss( omega[0] , omega[0] ) ;
	energy_margin -= __builtin_mulss( omega[2] , omega[2] ) ;
	if (( rmat[7]< 0 ) && ( energy_margin > 0 ))
	{
		return 0 ;
	}
	else
	{
		return 1 ;
	}	
#endif
}

float compute_tilt ( int16_t x , int16_t y , int16_t z )
{
	float xf , yf , zf ;
	float magnitude ;
	float angle ;
	xf = (float) x ;
	yf = (float) y ;
	zf = (float) z ;
	magnitude = sqrtf(xf*xf+yf*yf);
	angle = atan2f( magnitude , zf );
	return 57.296*angle ;
}

float tilt_angle()
{
#if ( HORIZONTAL_MOUNT == 1)
	return compute_tilt(rmat[6],rmat[7], rmat[8]);
#else
	return compute_tilt(rmat[6],rmat[8], -rmat[7]);
#endif
}