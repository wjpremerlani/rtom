/* 
 * File:   tiltLib.h
 * Author: bill
 *
 * Created on May 29, 2021, 1:30 PM
 */

#ifndef TILTLIB_H
#define	TILTLIB_H
#include <stdint.h>

int16_t too_much_tilt(int16_t degrees );
// degrees is maximum allowable tilt in integer degrees
// returns 1 if maximum tilt is exceeded, 0 otherwise
int16_t too_much_energy(int16_t degrees_per_second );
// degrees_per_second is maximum allowable energy in integer degrees per second
// returns 1 if maximum energy is is exceeded, 0 otherwise

extern void init_tilt_parameters( float , float , float ) ;
extern int16_t tilt_ok() ;

#endif	/* TILTLIB_H */

