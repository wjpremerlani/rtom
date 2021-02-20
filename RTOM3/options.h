
#define CUSTOM_OFFSETS // this is the best option for rockets. you will need to define offsets

#undef GROUND_TEST
#define GROUND_TEST ( 1 )
#define GYRO_RANGE ( 1000 )
#define CALIBRATION ( 1.0000 )

#define XACCEL_OFFSET	( 0 )
#define YACCEL_OFFSET	( 0 )
#define ZACCEL_OFFSET	( 0 )
#define XRATE_OFFSET	( 0 )
#define YRATE_OFFSET	( 0 )
#define ZRATE_OFFSET	( 0 )

////////////////////////////////////////////////////////////////////////////////
// Configure Input and Output Channels
//
// NUM_INPUTS: Set to 0-5 
#define NUM_INPUTS                          0

// NUM_OUTPUTS: Set to 3, 4, 5, or 6
#define NUM_OUTPUTS                         8


////////////////////////////////////////////////////////////////////////////////
// Serial Output BAUD rate for status messages
//  19200, 38400, 57600, 115200, 230400, 460800, 921600 // yes, it really will work at this rate
#define SERIAL_BAUDRATE                     19200 // default
