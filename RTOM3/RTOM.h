//	FBH	2021-04-18 

#ifndef RTOM_H
#define	RTOM_H

#include "../libDCM/libDCM_internal.h"
#include "../libDCM/mathlibNAV.h"
#include "../libUDB/heartbeat.h"

extern fractional rmat[];
extern int16_t launched ;

void rtom(void) ;
void rtom_init(void) ;

#endif
