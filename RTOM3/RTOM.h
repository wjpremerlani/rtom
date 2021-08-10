//	RTOM3 FBH Revisions
//	2021-03-04	Port dsPIC33FJ256GP710A-based board to dsPIC33FJ64GP206A-based board
//	2021-03-07	Delete some I/O defines from ConfigUDB5.h; move/create related defines here
//	2021-05-29  FBH Changes for RTOM3
//  2021-08-10  FBH  Changed energy refs to "motion"

#ifndef RTOM_H
#define	RTOM_H

#include "../libDCM/libDCM_internal.h"
#include "../libDCM/mathlibNAV.h"
#include "../libUDB/heartbeat.h"

extern fractional rmat[];
extern int16_t launched ;

extern int max_tilt ;
extern int max_motion ;

void rtom(void) ;
void rtom_init(void) ;

#endif
