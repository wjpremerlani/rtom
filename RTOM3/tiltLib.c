#include "tiltLib.h"
#include "options.h"
#include "../libDCM/libDCM.h"
#include "../libUDB/heartbeat.h"
#include "../libUDB/libUDB_internal.h"
#include "../libDCM/mathlibNav.h"
#include "tiltLib.h"

int16_t sine_max_tilt = 5460 ; // default max tilt is 30 degrees until init_tilt_parameters is called

void init_tilt_parameters ( float max_tilt , float max_tilt_rate , float look_back_time)
{
	sine_max_tilt = sine16( (uint16_t) 182.04*max_tilt) ;
}

void check_tilt()
{
	int32_t tilt_margin ;
	tilt_margin = __builtin_mulss( sine_max_tilt , sine_max_tilt ) ;
	
#if ( HORIZONTAL_MOUNT == 1)
	tilt_margin -= __builtin_mulss( rmat[6] , rmat[6] ) ;
	tilt_margin -= __builtin_mulss( rmat[7] , rmat[7] ) ;
	if (( rmat[8]> 0 ) && ( tilt_margin > 0 ))
	{
		LED_RED = LED_OFF ;
	}
	else
	{
		LED_RED = LED_ON ;
	}
#else
	tilt_margin -= __builtin_mulss( rmat[6] , rmat[6] ) ;
	tilt_margin -= __builtin_mulss( rmat[8] , rmat[8] ) ;
	if (( rmat[7]< 0 ) && ( tilt_margin > 0 ))
	{
		LED_RED = LED_OFF ; // inside cone
	}
	else
	{
		LED_RED = LED_ON ;  // excessive tilt
	}
#endif
	
}