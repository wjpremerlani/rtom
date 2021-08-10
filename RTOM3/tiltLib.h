/* 
 * File:   tiltLib.h
 * Author: bill
 *
 * Created on May 29, 2021, 1:30 PM
 * Updated  2021-07-25  FBH  Copied from Bill's branch
 * Updated  2021-08-10  FBH  Changed energy refs to "motion"
 */

#ifndef TILTLIB_H
#define	TILTLIB_H
#include <stdint.h>

int16_t too_much_tilt(int16_t degrees );
// degrees is maximum allowable tilt in integer degrees
// returns 1 if maximum tilt is exceeded, 0 otherwise

int16_t too_much_motion(int16_t degrees_per_second );
// degrees_per_second is maximum allowable motion in integer degrees per second
// returns 1 if maximum motion is is exceeded, 0 otherwise

float tilt_angle(void);

#endif	/* TILTLIB_H */