//
// branch of a branch
#define CUSTOM_OFFSETS

// fbh
#define LAUNCHER

#define HORIZONTAL_MOUNT ( 0 )

// Added by FBH for testing
#define ENVEL_TM_SELECT1
#define ENVEL_TM_SELECT2

#define GYRO_RANGE ( 1000 )
#define CALIBRATION ( 1.0000 )

#define XACCEL_OFFSET	( 0 )
#define YACCEL_OFFSET	( 0 )
#define ZACCEL_OFFSET	( 0 )
#define XRATE_OFFSET	( 0 )
#define YRATE_OFFSET	( 0 )
#define ZRATE_OFFSET	( 0 )

// NUM_OUTPUTS: Set to 3, 4, 5, or 6

//  FBH 2021-04-21
#define NUM_OUTPUTS                         3   // was 8

////////////////////////////////////////////////////////////////////////////////
// Serial Output BAUD rate for status messages
//  19200, 38400, 57600, 115200, 230400, 460800, 921600 // yes, it really will work at this rate
#define SERIAL_BAUDRATE                     19200 // default
