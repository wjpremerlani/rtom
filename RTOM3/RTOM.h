//	RTOM3 FBH Revisions
//	2021-03-04	Port dsPIC33FJ256GP710A-based board to dsPIC33FJ64GP206A-based board
//	2021-03-07	Delete some I/O defines from ConfigUDB5.h; move/create related defines here

#ifndef RTOM_H
#define	RTOM_H

#include "../libDCM/libDCM_internal.h"
#include "../libDCM/mathlibNAV.h"
#include "../libUDB/heartbeat.h"

// FBH - some RTOM3 related I/O
#define     BUZZER                  LATBbits.LATB5
#define     RELAY                   LATBbits.LATB4

#define     MANUAL_LAUNCH           _RG14  // note - RA2(SDA2), RA3(SCL2) n/a on 206A
#define     ENVEL_TIME_SELECT_1     _RC1
#define     ENVEL_TIME_SELECT_2     _RC2
#define     ANGLE_SELECT_1          _RB3
#define     ANGLE_SELECT_2          _RD9
#define     ANGLE_SELECT_3          _RD10
#define     RELAY_STATUS            _RD11

extern fractional rmat[];
extern int16_t launched ;

void rtom(void) ;
void rtom_init(void) ;

#endif
