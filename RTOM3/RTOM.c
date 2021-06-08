#include "RTOM.h"

// rmat[9] and launched are accessible, more variables can be added as needed
// rmat is the direction cosine matrix elements
// launched (accelerometers) indicates launch detection, it is equal to 0 prior to launch, 1 after launch
// launch can be simulated by pulling RA3 to ground - MANUAL LAUNCH


//	RTOM3 Version:
//	1.0	FBH	2021-04-18	initial structure roughly based on RTOM2 code
//  1.1 FBH 2021-05-23  added heartbeat
//  1.2 FBH 2021-05-26  add option select for dynamic mode

double firmware = 1.2 ;

signed char selected_angle ;

int beep ;
int	short_beep ;
int beep_count ;
int	pause_count ;
int long_pause ;
int beeps_done = 0 ;

int first_pass ;
int pass_count = 0 ;
int do_no_more ;
int relay_check = 0 ;
int relay_delay = 1 ;
int relay_closed = 0 ;
int relay_opened = 0 ;
int relay_check_done = 0 ;
int fatal_error = 0 ;

int dynamic_recovery_mode ;

int chirps ;
int chirp_pause ;
int chirp_length ;
int chirp_count ;

int rtom3_heartbeat ;
int heartbeat_pause = 0 ;
int heartbeat_length ;
int heartbeat_pause_count ;

long tilt_envelope ;
int excessive_Z_tilt ;
int excessive_tilt_latched ;

extern int rtom3_launched ;

//  INITIAL ONE-TIME
//  this function detects user's on-board jumper selections
//  it is called from main.c such that it only runs once, during the calibration period

void rtom_init(void)
{

//  ANGLE SELECT    
//  First, establish desired critical angle by determining setting of the RTOM3 on-board jumper pins;
//  respective beeps are assigned for audible feedback to the user
//  Formula for the selected angle = 16384 [0 degrees in 2.14 notation] * cos(selected_angle),
//  e.g., for (15 degrees) = 16384 * .966 = 15825
//	Uses binary number scheme, i.e., right-to-left

	if ((ANGLE_SELECT_JUMPER_1 == 0) && (ANGLE_SELECT_JUMPER_2 == 0) && (ANGLE_SELECT_JUMPER_3 == 0))
	{
		selected_angle = 5 ;			//	selected angles are expressed in degrees
		short_beep =1 ;
		tilt_envelope = 16318 ;			//	cos 5d = .996
	}

	else if ((ANGLE_SELECT_JUMPER_1 == 0) && (ANGLE_SELECT_JUMPER_2 == 0) && (ANGLE_SELECT_JUMPER_3 == 1))
	{
		selected_angle = 10 ;
		beep = 1 ;
		tilt_envelope = 16138 ;			// cos 10d = .985
	}

	else if ((ANGLE_SELECT_JUMPER_1 == 0) && (ANGLE_SELECT_JUMPER_2 == 1) && (ANGLE_SELECT_JUMPER_3 == 0))
	{
		selected_angle = 15 ;
		beep = 1 ;
		short_beep = 1 ;
		tilt_envelope = 15825 ;			// cos 15d = .966
	}

	else if ((ANGLE_SELECT_JUMPER_1 == 0) && (ANGLE_SELECT_JUMPER_2 == 1) && (ANGLE_SELECT_JUMPER_3 == 1))
	{
		selected_angle = 20 ;			
		beep = 2 ;
		tilt_envelope = 15396 ;			// cos 20d = .940
	}

    else if ((ANGLE_SELECT_JUMPER_1 == 1) && (ANGLE_SELECT_JUMPER_2 == 0) && (ANGLE_SELECT_JUMPER_3 == 0))
	{
		selected_angle = 25 ;
		beep = 2 ;	
		short_beep = 1 ;
		tilt_envelope = 14843 ;			// cos 25d = .906
	}

	else if ((ANGLE_SELECT_JUMPER_1 == 1) && (ANGLE_SELECT_JUMPER_2 == 0) && (ANGLE_SELECT_JUMPER_3 == 1))
	{
		selected_angle = 30 ;
		beep = 3 ;
		tilt_envelope = 14189 ;			// cos 30d = .866
	}

	else if ((ANGLE_SELECT_JUMPER_1 == 1) && (ANGLE_SELECT_JUMPER_2 == 1) && (ANGLE_SELECT_JUMPER_3 == 0))
	{
		selected_angle = 35 ;
		beep = 3 ;
		short_beep = 1 ;
		tilt_envelope = 13418 ;			// cos 35d = .819
	}

	else if ((ANGLE_SELECT_JUMPER_1 == 1) && (ANGLE_SELECT_JUMPER_2 == 1) && (ANGLE_SELECT_JUMPER_3 == 1))
	{
		selected_angle = 40 ;
		beep = 4 ;
		tilt_envelope = 12550 ;			// cos 40d = .766
	}

//  DYNAMIC OPTION SELECT    
// then, setup dynamic mode option selection
	if (OPTION_SELECT_JUMPER_1 == 1)
	{
        dynamic_recovery_mode = 1 ;             // dynamic recovery mode
	}
	else
	{
        dynamic_recovery_mode = 0 ;             // basic recovery mode
	}

    return ;

}

//  MAIN ROUTINE  //
void rtom(void)
// RTOM3 code - gets called 40 times per second
{

//  RELAY LED FOLLOW    
// set up for relay contact following with the blue LED
    if (RELAY_POSITION == RELAY_CLOSED)
    {
        LED_BLUE = LED_ON ;
    }
    else
    {
        LED_BLUE = LED_OFF ;
    }

//  RELAY CYCLE TEST
// cycle the relay to check to see if operational
// if get fatal error, user alert chirps  
    
    if ( relay_delay < 40 )         // pause 1 sec to allow relay to close
	{
        RELAY = CLOSE_RELAY ;    
		relay_delay++ ;
	}
        
    if ( relay_delay >= 40 && relay_delay < 80 )
    {        
        if ( RELAY_POSITION == RELAY_CLOSED && relay_delay == 40 )
        {
            relay_closed = 1 ;
            relay_delay++ ;                
        }
        else
        {
            RELAY = OPEN_RELAY ;
            relay_delay++ ;
        }    
    }
        
   	if ( relay_delay == 80 )
    {
        if (RELAY_POSITION == RELAY_OPEN )
  		{
            relay_opened = 1 ;
            relay_delay++ ;
            relay_check_done = 1 ;
        }
    else
        {
            relay_check_done = 1 ;
        }
    }    
    
    if ( relay_check_done == 1)
    {
        if ( relay_closed == 1 && relay_opened == 1 )
        {
            fatal_error = 0 ;
        }
        else
        {
            fatal_error = 1 ;
        }    
    }

//  RELAY-ERROR TEST FEEDBACK-ABORT
// here we check to see the result of the relay contact closure test above
// if test fails, abort operation and sound a continuous series of chirps    
    if (( fatal_error == 1) || (do_no_more == 1))
		{
			do_no_more = 1 ;															// Locks it into chirp mode

			if ( chirp_pause == 0 )														// Sets number of fatal error code beeps
			{
				chirps = 1 ;
				chirp_pause = 1 ;
			}

			if ( chirps > 0 )															// Then, process the chirps
			{
				if ( chirp_length < 12 )
				{
					TONER = TONER_ON ;
					chirp_length++ ;
				}
				else if ( chirp_length == 12 )
				{
					if ( pause_count < 12 )
					{
						TONER = 0 ;
						pause_count++ ;
					}
					else if ( pause_count == 12 )
					{
						chirps-- ;
						chirp_length = 0 ;
						pause_count = 0 ;
					}
				}
			}
		else if ( chirp_pause == 1 )                                                    // Then, generate a pause before
			{																			// chirps resound
				if (pause_count < 12)
				{
					TONER = TONER_OFF ;
					pause_count++ ;
				}
				else if (pause_count == 12)
				{
					pause_count = 0 ;
					chirp_pause = 0 ;
				}
			}
		}

//  ANGLE FEEDBACK BEEPS    
// then, if the check on the relay determines it cycled OK, the beeper will now beep
// sounds according to the above angle selection choice (one long for each 10 degrees,
// one short for 5 degrees), and then sound for options jumper selections

// This first sequence beeps the angle selection determined above; disable this routine if fatal error
    if ( beep > 0 && fatal_error == 0 && relay_check_done == 1 )
	{
		if ( beep_count < 40 )												// Process any long angle beeps
		{
			TONER = TONER_ON ;
			beep_count++ ;
		}
		else if ( beep_count == 40 )
		{
			if ( pause_count < 20 )
			{
				TONER = TONER_OFF ;
				pause_count++ ;
			}
			else if ( pause_count == 20 )
			{
				beep-- ;
				beep_count = 0 ;
				pause_count = 0 ;
			}
		}
	}
    
	else if ( short_beep == 1 && fatal_error == 0 && relay_check_done == 1 )	// Then do short angle beeps
	
    {
		first_pass = 0 ;														// See above
		if ( beep_count < 10 )
		{
			TONER = TONER_ON ;
			beep_count++ ;
		}
		else if ( beep_count == 10 )
		{
			if ( pause_count < 20 )
			{
				TONER = TONER_OFF ;
				pause_count++ ;
			}
			else if ( pause_count == 20 )
			{
				short_beep-- ;
				beep_count = 0 ;
				pause_count = 0 ;
                long_pause = 1 ;
			}
		}
	}
	else if ( long_pause == 1 && fatal_error == 0 && relay_check_done == 1 )	// Finally, generate a pause before
	{																			// options tones sound to emphasize
		if (pause_count < 50)													// the distinction
		{
			TONER = TONER_OFF ;
			pause_count++ ;
		}
		else if (pause_count == 50)
		{
			pause_count = 0 ;
			long_pause = 0 ;
			beeps_done = 1 ;
		}
	}

//  HEARTBEAT    
// if all is well, set up a heartbeat tone that sounds every 5 seconds to indicate to the user
// that the RTOM3 is still functioning OK;
// different beep count for basic recovery mode and for dynamic recovery mode    
// turn it off after launch to conserve battery
    
    if ((beeps_done == 1) && (launched == 0))
		{
			if ( heartbeat_pause == 0 )
			{
				if ( dynamic_recovery_mode == 0 )					// Sets number of beeps
                {
                    rtom3_heartbeat = 2 ;
                }
                else
                {
                    rtom3_heartbeat = 3 ;
                }    
				heartbeat_pause = 1 ;
			}

			if ( rtom3_heartbeat > 0 )								// Then, process the beeps
			{
				if ( heartbeat_length < 3 )
				{
					TONER = TONER_ON ;
					heartbeat_length++ ;
				}
				else if ( heartbeat_length == 3 )
				{
					if ( heartbeat_pause_count < 3 )
					{
						TONER = 0 ;
						heartbeat_pause_count++ ;
					}
					else if ( heartbeat_pause_count == 3 )
					{
						rtom3_heartbeat-- ;
						heartbeat_length = 0 ;
						heartbeat_pause_count = 0 ;
					}
				}
			}
            else if ( heartbeat_pause == 1 )                                                 // Then, generate a pause before
			{																			     // beeps resound
				if (heartbeat_pause_count < 200)
				{
					TONER = TONER_OFF ;
					heartbeat_pause_count++ ;
				}
				else if (heartbeat_pause_count == 200)
				{
					heartbeat_pause_count = 0 ;
					heartbeat_pause = 0 ;
				}
			}
		}

    
//	CORE TILTOMETER FUNCTIONS
//	This function uses the DCM (Direction Cosine Matrix) to
//	provide an angle off vertical (earth Z axis) using rmat[7]);
//  then it monitors that angle for a defined excess-tilt indication;
//  the relay is maintained closed to allow ignition signal to pass through
//  unless angle is exceeded, whereby the relay is opened to inhibit ignition
//  if rocket has been launched, either manually or real flight, and angle has
//  been exceeded, the relay will not return closed if angle returns to inside the cone    
    
    if ( fatal_error == 0 && relay_check_done == 1 && excessive_tilt_latched == 0 )
    {
    	if ( rmat[7] < - tilt_envelope )   // Compares tilt to the selected critical angle;
     	{								   // if rocket tilt is less than the critical angle:
    		RELAY = CLOSE_RELAY ;		   // relay closes to allow ignition
//    		excessive_Z_tilt = 0 ;		   // and assigns the excessive tilt logic as FALSE
    	}
        else
        {								   // if the critical angle is exceeded:

            RELAY = OPEN_RELAY ;		   // relay opens to prohibit ignition
                                           // the blue LED follows the relay contacts
                                           // on for closed and off for open

//            excessive_Z_tilt = 1 ;	   // this assigns excessive tilt logic as TRUE

            if ( launched == 1 )		   // The following variable will not latch unless rocket has been launched
            {							   // - this allows rocket movement during pre-launch calib.
            excessive_tilt_latched = 1 ;   // If set, allows only one-shot at ignition - precludes
            }							   // ignition if rocket wanders back into the envelope during flight.
        }
    }


/*
//	For various functions including serial output for logging - 
//	some of these are additional factors to use when we decide to monitor the rate of tilt angle change

	rollPitchVector.x = rmat[6] ;
	rollPitchVector.y = rmat[7] ;
	rect_to_polar( &rollPitchVector ) ;				// this computes sqrt(rmat[6]**2+rmat[7]**2)
	sineTilt = rollPitchVector.x ;					// 16 bit value = RMAX*sin(tiltangle), tiltangle in radians
													// will track only from 0 to 90 degrees, is very accurate
	
	cosTilt = rmat[8] ;								// 16 bit value = RMAX*cos(tiltangle), tiltangle in radians
	rateSineTilt = (sineTilt - prevSineTilt)*40 ;	// 16 bit value = d/dt of sineTilt
													// very good indicator of the rate of tilt from 0 to 90 degrees
	
	prevSineTilt = sineTilt ;						// used to compute d/dt
	rollPitchVector.x = cosTilt ;
	rollPitchVector.y = sineTilt ;
	tilt_16 = rect_to_polar16( &rollPitchVector ) ;	// 16 bit tilt angle, measured in "byte circular";
													// straight up is 0, straight down is 128*256
													// will track from 0 to 180 degrees
													// use of 16 bt rect to polar yields good accuracy	    
*/        
	return ;
    
}
