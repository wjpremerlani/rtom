//	RTOM3 FBH Revisions
//	2021-03-04	Port dsPIC33FJ256GP710A-based board to dsPIC33FJ64GP206A-based board
//  2021-03-07  Move a couples defines to RTOM.h
//  2021-05-29  Changes for RTOM3

#define CUSTOM_OFFSETS

#define HORIZONTAL_MOUNT ( 1 )

#define GYRO_RANGE ( 1000 )
#define CALIBRATION ( 0.9885 )

#define XACCEL_OFFSET	( 0 )
#define YACCEL_OFFSET	( 0 )
#define ZACCEL_OFFSET	( 0 )
#define XRATE_OFFSET	( 0 )
#define YRATE_OFFSET	( 0 )
#define ZRATE_OFFSET	( 0 )

// NUM_OUTPUTS: Set to 3, 4, 5, or 6
//  FBH 2021-05-29
#define NUM_OUTPUTS                         3   // was 8

////////////////////////////////////////////////////////////////////////////////
// Serial Output BAUD rate for status messages
//  19200, 38400, 57600, 115200, 230400, 460800, 921600 // yes, it really will work at this rate
#define SERIAL_BAUDRATE                     19200 // default
