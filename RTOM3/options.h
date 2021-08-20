//	RTOM3 FBH Revisions
//	2021-03-04	Port dsPIC33FJ256GP710A-based board to dsPIC33FJ64GP206A-based board
//  2021-03-07  Move a couples defines to RTOM.h
//  2021-05-29  Changes for RTOM3
//  2021-07-28  Bill updating the gyro calib factor based upon new testing
//  2021-08-13   "

//  RTOM3 WJP Revisions
//  1.0 WJP 2021-08-20  Added two new options: silence the "toner", output Bill's gyro testing data

#define GYRO_RANGE ( 1000 )
#define CALIBRATION ( 0.9885 )  // was 1.0000; 0.9863

/// Serial Output BAUD rate for transfer of data to OpenLog:
#define SERIAL_BAUDRATE                     19200 // default

//  development and debugging options
#define HORIZONTAL_MOUNT    ( 0 ) // set to 0 for vertical mount, 1 for horizontal mount
#define TONER_ENABLED	    ( 1 ) // set to 1 to enable toner, 0 to silence it
#define STANDARD_OUTPUT	    ( 1 ) // set to 1 for Frank's output format, 0 for Bill's

#define CUSTOM_OFFSETS // must use this option for RTOM3
#define XACCEL_OFFSET	( 0 )
#define YACCEL_OFFSET	( 0 )
#define ZACCEL_OFFSET	( 0 )
#define XRATE_OFFSET	( 0 )
#define YRATE_OFFSET	( 0 )
#define ZRATE_OFFSET	( 0 )

// NUM_OUTPUTS: Set to 3, 4, 5, or 6
//  FBH 2021-05-29
#define NUM_OUTPUTS                         3   // was 8

