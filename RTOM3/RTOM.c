#include "RTOM.h"
#include "tiltLib.h"

// rmat[9] and launched are accessible, more variables can be added as needed
// rmat is the direction cosine matrix elements
// launched (accelerometers) indicates launch detection, it is equal to 0 prior to launch, 1 after launch
// launch can be simulated by pulling either SCL or SDA pins to ground

//  2021-05-29  FBH Changes for RTOM3


//	RTOM3 Version:
//	0.0	FBH	2021-04-18	initial structure roughly based on RTOM2 code
//  0.1 FBH 2021-05-23  added heartbeat
//  0.2 FBH 2021-05-26  add option select for dynamic mode
//  0.3 FBH 2021-06-08  corrected issue where heartbeat skipped if any mod10 angle was selected
//  0.4 WJP 2021-06-10  merged the new tilt detection algorithm
//  0.5 FBH 2021-07-23  re-define option selections; implement new REENTRY mode; set up for end-user logger output
//  0.6 FBH 2021-08-06  implement latest operating modes; revise logger output
//  0.7 FBH 2021-08-08  revise PRE_LAUNCH and other states to assure relay position is what we want per Bill comments
//  0.8 FBH 2021-08-10  change "energy" and "debounce" labels to "motion"; change operating mode and angle select numbering to reflect
//                      most-to-least "restrictive" level notion; incorporate applying delays by selected angle - bigger angle gets longer delay


double firmware = 0.7 ;

int max_tilt ;
int tilt_flag = 0 ;
int max_motion ;
int motion_flag = 0 ;

#define PRE_LAUNCH 1
#define NO_MOTION 2
#define REENTRY 3
#define MOTION 4
#define LOCKOUT 5

float reentry_delay_time ;
float reentry_delay_count ;
int reentry_delay ;
float motion_delay_time ;
float motion_delay_count ;
int motion_delay ;
int motion_wait ;
int state = 1 ;

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

int reentry_mode = 0 ;
int abort_mode = 0 ;
int motion_mode = 0 ;
int option_mode ;

int logger_launch ;
int logger_startup ;

int chirps ;
int chirp_pause ;
int chirp_length ;
int chirp_count ;

int rtom3_heartbeat ;
int heartbeat_pause = 0 ;
int heartbeat_length ;
int heartbeat_pause_count ;
int oper_mode ;

long tilt_envelope ;

int ignition_dis ;


//  this function detects user's on-board jumper selections
//  it is called from main.c such that it only runs once, during the calibration period

void rtom_init(void)
{
    
//  First, establish desired critical angle by determining setting of the RTOM3 on-board jumper pins;
//  respective beeps are assigned for audible feedback to the user
//  note: that no jumpers is set to 30 degrees rather than the lowest of 10 - no jumpers will be the default for all options
//  note: jumper call outs are as labeled on the RTOM3 board
// ANGLE SELECTION
	if ((JUMPER_1 == 0) && (JUMPER_2 == 0))
	{
		max_tilt = 10 ;                 //	DEFAULT critical angle - i.e., no jumpers == 30
		max_motion = 50 ;       		//  degrees per second
		reentry_delay_time = 0.50 ;  	//  seconds
		motion_delay_time = 0.50 ;      //  seconds
		beep = 1 ;
	}

	else if ((JUMPER_1 == 0) && (JUMPER_2 == 1))
	{
		max_tilt = 15 ;                 //	critical angle
		max_motion = 50 ;       		//  degrees per second
		reentry_delay_time = 0.75 ;  	//  seconds
		motion_delay_time = 0.75 ;      //  seconds
		beep = 1 ;
        short_beep = 1 ;
	}

	else if ((JUMPER_1 == 1) && (JUMPER_2 == 0))
	{
		max_tilt = 20 ;                 //	critical angle
		max_motion = 50 ;       		//  degrees per second
		reentry_delay_time = 1.0 ;  	//  seconds
		motion_delay_time = 1.0 ;       //  seconds
		beep = 2 ;
	}

	else if ((JUMPER_1 == 1) && (JUMPER_2 == 1))
	{
		max_tilt = 30 ;                 //	critical angle
		max_motion = 50 ;       		//  degrees per second
		reentry_delay_time = 1.50 ;  	//  seconds
		motion_delay_time = 1.50 ;      //  seconds
		beep = 3 ;
	}

// OPERATING MODE SELECTION    
// then, setup operating mode option selection
	if ((JUMPER_3 == 0) && (JUMPER_4 == 0))          // Mode 1 - ABORT
	{
        abort_mode = 1 ;                             // could keep simple and use either abort or reentry mode, but using both this keeps things more clear later on
        reentry_mode = 0 ;
        motion_mode = 1 ;
        option_mode = 1 ;
	}
	else if ((JUMPER_3 == 0) && (JUMPER_4 == 1))     // Mode 2 - ABORT WITH MOTION
	{
        abort_mode = 1 ;
        reentry_mode = 0 ;
        motion_mode = 0 ;
        option_mode = 2 ;
	}        
	else if ((JUMPER_3 == 1) && (JUMPER_4 == 0))     // Mode 3 - REENTRY
	{
        abort_mode = 0 ;
        reentry_mode = 1 ;
        motion_mode = 1 ;
        option_mode = 3 ;
	}             
	else if ((JUMPER_3 == 1) && (JUMPER_4 == 1))     // Mode 4 - REENTRY WITH MOTION
	{
        abort_mode = 0 ;
        reentry_mode = 1 ;
        motion_mode = 0 ;
        option_mode = 4 ;
	}             
    
    return ;
}

void rtom(void)
// RTOM3 code - gets called 40 times per second
{
    
// set up for relay contact following with the blue LED and flag for logger
    if (RELAY_POSITION == RELAY_CLOSED)
    {
        LED_BLUE = LED_ON ;
        ignition_dis = 0 ;
    }
    else
    {
        LED_BLUE = LED_OFF ;
        ignition_dis = 1 ;
    }

    
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

    
// then, if the check on the relay determines it cycled OK, the beeper will now beep
// sounds according to the above angle selection choice (one long for each 10 degrees,
// one short for 5 degrees)

// This first sequence indicates the angle selection determined above; disabled if fatal error
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
                long_pause = 1 ;
			}
		}
	}

	else if ( short_beep == 1 && fatal_error == 0 && relay_check_done == 1 )	// Then do short angle beeps
    {
		first_pass = 0 ;
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

    
// if all is well, set up a heartbeat tone that sounds every 5 seconds to indicate RTOM3 is still functioning;    
// turn it off after launch to conserve battery
    
    if ((beeps_done == 1) && (launched == 0))
		{
			if ( heartbeat_pause == 0 )
			{
                switch (option_mode)                                // check mode and assign number of chirps
                {    
                        case 1 :
                        rtom3_heartbeat = 1; break;
                        case 2 :
                        rtom3_heartbeat = 2; break;
                        case 3 :
                        rtom3_heartbeat = 3; break;
                        case 4 :
                        rtom3_heartbeat = 4; break;
                        default : break ;
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
//	monitor a selected critical angle (jumpers) off vertical (earth Z axis)
//  to provide an excess-tilt indication;
//  the ignition relay is maintained closed to allow an ignition signal to pass through
//  unless angle is exceeded, whereby the relay is opened to inhibit ignition
//  if rocket has been launched, either manually or real flight, and angle has
//  been exceeded, the relay will not return closed if angle returns to inside the cone
//  if operating mode has been selected as ABORT; if mode is REENTRY, certain conditions will
//  re-enable ignition    
    
    tilt_flag = too_much_tilt(max_tilt) ;                                               // 1 = outside envelope - not OK
    motion_flag = too_much_motion(max_motion) ;                                         // 1 = high motion - not OK
    reentry_delay = (uint16_t) 40.0 * reentry_delay_time ;
    motion_delay = (uint16_t) 40.0 * motion_delay_time ;
		
    if ( fatal_error == 0 && relay_check_done == 1 )
    {
        switch (state)
        {
            case PRE_LAUNCH :                                                           // PRE-LAUNCH; no lockout; relay will cycle if rocket tilted past selected_angle; one-time state;
				if (tilt_flag == 0)                                                     // this check allows user to test at bench; rocket inside envelope
                {
					RELAY = CLOSE_RELAY ;
                }    
				else if (tilt_flag == 1)                                                // rocket outside envelope
                {    
					RELAY = OPEN_RELAY ;
                }
                
				if ((launched == 1) && (motion_mode == 0))                              // rocket launched; route to proper motion selection, or, lockout if excess tilt
                {
					if (tilt_flag == 0)                                                 // rocket inside envelope at launch
                    {
					RELAY = CLOSE_RELAY ;
                    state = NO_MOTION ;
                    }    
                    else                                                                // rocket outside envelope at launch
                    {    
					RELAY = OPEN_RELAY ;
                    state = LOCKOUT ;
                    }
                    break ;
                }
                else if ((launched == 1) && (motion_mode == 1))
                {
                    if (tilt_flag == 0)                                                 // rocket inside envelope at launch
                    {
					RELAY = CLOSE_RELAY ;
                    state = MOTION ;
                    }    
                    else                                                                // rocket outside envelope at launch
                    {    
					RELAY = OPEN_RELAY ;
                    state = LOCKOUT ;
                    }
                    break ;
                }
                break ;
				
            case NO_MOTION :                                                            // initial state of launched rocket if no motion monitor; loops while waiting for excess tilt
                if (tilt_flag == 1)                                                     // rocket outside envelope - lockout
                {
                    RELAY = OPEN_RELAY ;
                    state = LOCKOUT ;
                }
                break ;
                
            case REENTRY :                                                              // rocket reenters envelope; stays locked out for reentry delay time; motion monitor not in place
                if (tilt_flag == 1)                                                     // should rocket again go outside envelope - lockout
                {
                    RELAY = OPEN_RELAY ;
                    state = LOCKOUT ;
                    break ;
                }
                
                if ( reentry_delay_count > 0 )                                          // rocket back inside envelope, but keep relay open until reentry delay consumed
                {
                    reentry_delay_count -- ;
                }
                else
                {
                    RELAY = CLOSE_RELAY ;                                               // rocket remained inside envelope for reentry delay, so move to proper motion state
                    motion_delay_count = 0 ;
                    if (motion_mode == 1)
                    {
                        state = MOTION ;
                    }
                    else
                    {
                        state = NO_MOTION ;
                    }
                    break ; 
                }
                
                break ;
                
            case MOTION :                                                               // initial state of launched rocket if motion monitor is selected; if motion excessive, motion_delay is tirggered
                if (tilt_flag == 1)                                                     // should rocket go outside envelope - lockout
                {
                    RELAY = OPEN_RELAY ;
                    state = LOCKOUT ;
                    break ;
                }
                
                if (motion_flag == 1)                                                   // monitor motion - if excessive, disable ignition and reset the motion count
                {    
                    RELAY = OPEN_RELAY ;
                    motion_delay_count = motion_delay ;
                }
                
                if (motion_flag == 0 )                                                  // monitor motion - if safe, check for need to motion
                {
                    if ( motion_delay_count > 0 )                                       // keep relay open until motion delay is consumed
                    {
                        RELAY = OPEN_RELAY ;
                        motion_delay_count -- ;
                    }
                    else
                    {
                        RELAY = CLOSE_RELAY ;                                           // rocket remained inside envelope for motion delay, so continue to monitor motion
                        break ;
                    }    
                }                            

                break ;
                
            case LOCKOUT :                                                              // LOCKOUT; monitors tilt
                RELAY = OPEN_RELAY ;                                                    // should be redundant, but just in case
                if (tilt_flag == 0)                                                     // monitors for return to inside the envelope; if tilt remains excessive, loop
                {
                    if (reentry_mode == 1)                                              // if tilt OK now, if REENTRY is selected mode, keeps relay open and moves puck to reentry state
                    {                                                                   // or, if abort mode is selected, just breaks out and loops - no reentry allowed
                        state = REENTRY ;
                        reentry_delay_count = reentry_delay ;                           // resets the reentry timer
                        break ;
                    }
                    else                                                                // lockout persists if selected mode is ABORT
                    {
                        break ;                                                         // just loop to keep relay open
                    } 
                }
                break ;
                
            default :
                break ;
        }
    }
      
	return ;
}
