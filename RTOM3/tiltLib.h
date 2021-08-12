/* 
 * File:   tiltLib.h
 * Author: bill
 *
 * Created on May 29, 2021, 1:30 PM
 */

#ifndef TILTLIB_H
#define	TILTLIB_H
#include <stdint.h>


extern void init_tilt_parameters( float , float , float ) ;
extern int16_t tilt_ok() ;
extern double tilt_angle() ;

#endif	/* TILTLIB_H */

