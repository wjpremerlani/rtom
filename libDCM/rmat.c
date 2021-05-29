// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2011 MatrixPilot Team
// See the AUTHORS.TXT file for a list of authors of MatrixPilot.
//
// MatrixPilot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixPilot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixPilot.  If not, see <http://www.gnu.org/licenses/>.

//	FBH	2021-05-29 changes for RTOM3


#include "libDCM_internal.h"
#include "mathlibNAV.h"
#include "../libUDB/heartbeat.h"

//	RTOM3	FBH	2021-04-18	access RTOM3 defines
//#include "RTOM.h"

// These are the routines for maintaining a direction cosine matrix
// that can be used to transform vectors between the earth and plane
// coordinate systems. The 9 direction cosines in the matrix completely
// define the orientation of the plane with respect to the earth.
// The inverse of the matrix is equal to its transpose. This defines
// the so-called orthogonality conditions, which impose 6 constraints on
// the 9 elements of the matrix.

// All numbers are stored in 2.14 format.
// Vector and matrix libraries work in 1.15 format.
// This combination allows values of matrix elements between -2 and +2.
// Multiplication produces results scaled by 1/2.

#define RMAX15 24576 //0b0110000000000000   // 1.5 in 2.14 format

#define GGAIN CALIBRATION*SCALEGYRO*6*(RMAX*(1.0/HEARTBEAT_HZ)) // integration multiplier for gyros
fractional ggain[] =  { GGAIN, GGAIN, GGAIN };

uint16_t spin_rate = 0;
//fractional spin_axis[] = { 0, 0, RMAX };

#if ( BOARD_TYPE == UDB5_BOARD)
// modified gains for MPU6000
// stock gains:
//#define KPROLLPITCH (ACCEL_RANGE * 1280/3)
//#define KIROLLPITCH (ACCEL_RANGE * 3400 / HEARTBEAT_HZ)

// rocket gains:

//#define KPROLLPITCH (ACCEL_RANGE * 900)
//#define KIROLLPITCH ((uint32_t) ACCEL_RANGE * (uint32_t) 10240 / (uint32_t) HEARTBEAT_HZ)

//#define KPROLLPITCH (ACCEL_RANGE * 1800)
//#define KPROLLPITCH ( 4096 )
#define KPROLLPITCH ( 2*2048 )
//#define KIROLLPITCH ((uint32_t) ACCEL_RANGE * (uint32_t) 40960 / (uint32_t) HEARTBEAT_HZ)
//#define KIROLLPITCH ((uint32_t) ACCEL_RANGE * (uint32_t) 2560 / (uint32_t) HEARTBEAT_HZ)
#define KIROLLPITCH ( (uint32_t) 4*2560 / (uint32_t) HEARTBEAT_HZ)

#elif (BOARD_TYPE == UDB4_BOARD)
// Paul's gains for 6G accelerometers
#define KPROLLPITCH (256*5)
#define KIROLLPITCH (10240/HEARTBEAT_HZ) // 256

#else
#error Unsupported BOARD_TYPE
#endif // BOARD_TYPE

//#define KPYAW 256*4
//#define KPYAW ( 4096 )
#define KPYAW ( 2*2048 )
//#define KIYAW 32
//#define KIYAW (1280/HEARTBEAT_HZ)
#define KIYAW (4*2560/HEARTBEAT_HZ)

#define GYROSAT 15000
// threshold at which gyros may be saturated

fractional rmat[9] = {RMAX , 0 , 0 , 0 , RMAX , 0 , 0 , 0 , RMAX };

// rmat is the matrix of direction cosines relating
// the body and earth coordinate systems.
// The columns of rmat are the axis vectors of the plane,
// as measured in the earth reference frame.
// The rows of rmat are the unit vectors defining the body frame in the earth frame.
// rmat therefore describes the body frame B relative to the Earth frame E
// and in Craig's notation is represented as (B->E)R: LateX format: presupsub{E}{B}R
// To transform a point from body frame to Earth frame, multiply from the left
// with rmat.

// gyro rotation vector:
fractional omegagyro[] = { 0, 0, 0 };
fractional omega[] = { 0, 0, 0 };

// gyro correction vectors:
static fractional omegacorrP[] = { 0, 0, 0 };
fractional omegacorrI[] = { 0, 0, 0 };

// correction vector integration;
static union longww gyroCorrectionIntegral[] =  { { 0 }, { 0 },  { 0 } };

// accumulator for computing adjusted omega:
fractional omegaAccum[] = { 0, 0, 0 };

// gravity, as measured in plane coordinate system
fractional gplane[] = { 0, 0, GRAVITY };

// horizontal velocity over ground, as measured by GPS (Vz = 0)
fractional dirOverGndHGPS[] = { 0, RMAX, 0 };

// horizontal direction over ground, as indicated by Rmatrix
fractional dirOverGndHrmat[] = { 0, RMAX, 0 };

// vector buffer
static fractional errorRP[] = { 0, 0, 0 };
static fractional errorYawground[] = { 0, 0, 0 };
static fractional errorYawplane[]  = { 0, 0, 0 };

// measure of error in orthogonality, used for debugging purposes:
static fractional error = 0;

static inline void read_gyros(void)
{
	// fetch the gyro signals and subtract the baseline offset, 
	// and adjust for variations in supply voltage
	omegagyro[0] = XRATE_VALUE;
	omegagyro[1] = YRATE_VALUE;
	omegagyro[2] = ZRATE_VALUE;
}

inline void read_accel(void)
{
	gplane[0] = XACCEL_VALUE;
	gplane[1] = YACCEL_VALUE;
	gplane[2] = ZACCEL_VALUE;
}

void udb_callback_read_sensors(void)
{
	read_gyros(); // record the average values for both DCM and for offset measurements
	read_accel();
}

fractional theta[3];
// The update algorithm!!
static void rupdate(void)
{
	// This is the key routine. It performs a small rotation
	// on the direction cosine matrix, based on the gyro vector and correction.
	// It uses vector and matrix routines furnished by Microchip.
	fractional delta_angle[9];
	fractional delta_angle_square_over_2[9];
	fractional delta_angle_cube_over_6[9];
	fractional rup[9];

	fractional rbuff[9];
		
	VectorAdd(3, omegaAccum, omegagyro, omegacorrI);
	VectorAdd(3, omega, omegaAccum, omegacorrP);
	//	scale by the integration factors:
	VectorMultiply(3, theta, omega, ggain); // Scalegain of 2
	// diagonal elements of the update matrix:
	rup[0] = RMAX;
	rup[4] = RMAX;
	rup[8] = RMAX;
	rup[1] = 0 ;
	rup[2] = 0 ;
	rup[3] = 0 ;
	rup[5] = 0 ;
	rup[6] = 0 ;
	rup[7] = 0 ;

	// construct the delta angle matrix:
	delta_angle[0] = 0 ;
	delta_angle[1] = -theta[2];
	delta_angle[2] =  theta[1];
	delta_angle[3] =  theta[2];
	delta_angle[4] = 0 ;
	delta_angle[5] = -theta[0];
	delta_angle[6] = -theta[1];
	delta_angle[7] =  theta[0];
	delta_angle[8] = 0 ;
	
	// compute 1/2 of square of the delta angle matrix
	// since a matrix multiply divides by 2, we get it for free	
	MatrixMultiply( 3, 3, 3, delta_angle_square_over_2 , delta_angle , delta_angle );
	
	// first step in computing delta angle cube over 6, compute it over 4 ;
	MatrixMultiply( 3, 3, 3, delta_angle_cube_over_6 , delta_angle_square_over_2 , delta_angle );
	
	// multiply by 2/3
	int16_t loop_index ;
	for ( loop_index = 0 ; loop_index <= 8 ; ++ loop_index ) 
	{
		delta_angle_cube_over_6[loop_index] = __builtin_divsd(__builtin_mulsu(delta_angle_cube_over_6[loop_index],2 ),3);
	}
	
	// form the update matrix
	MatrixAdd(3, 3, rup, rup, delta_angle );
	MatrixAdd(3, 3, rup, rup, delta_angle_square_over_2 );
	MatrixAdd(3, 3, rup, rup, delta_angle_cube_over_6 );

	// matrix multiply the rmatrix by the update matrix
	MatrixMultiply(3, 3, 3, rbuff, rmat, rup);
	// multiply by 2 and copy back from rbuff to rmat:
	MatrixAdd(3, 3, rmat, rbuff, rbuff);
}

// The normalization algorithm
static void normalize(void)
{
	//  This is the routine that maintains the orthogonality of the
	//  direction cosine matrix, which is expressed by the identity
	//  relationship that the cosine matrix multiplied by its
	//  transpose should equal the identity matrix.
	//  Small adjustments are made at each time step to assure orthogonality.

	fractional norm;    // actual magnitude
	fractional renorm;  // renormalization factor
	fractional rbuff[9];

	// take the tilt row without any adjustments
	VectorCopy( 3 , &rbuff[6] , &rmat[6] ) ;
	// compute the negative of the dot product between rows 2 and 3
	error =  - 2*VectorDotProduct(3, &rmat[3], &rmat[6]);
	// compute adjustment to row 2 that will make it more perpendicular to row 3
	VectorScale( 3 , &rbuff[3] , &rbuff[6] , error ) ;
	VectorAdd( 3, &rbuff[3] , &rbuff[3] , &rmat[3] ) ;
	// use the cross product of row 2 and 3 to get the first row
	VectorCross(&rbuff[0] , &rbuff[3] , &rbuff[6] ) ;

	// Use a Taylor's expansion for 1/sqrt(X*X) to avoid division in the renormalization

	// rescale row1
	norm = VectorPower(3, &rbuff[0]); // Scalegain of 0.5
	renorm = RMAX15 - norm;
	VectorScale(3, &rbuff[0], &rbuff[0], renorm);
	VectorAdd(3, &rmat[0], &rbuff[0], &rbuff[0]);
	// rescale row2
	norm = VectorPower(3, &rbuff[3]);
	renorm = RMAX15 - norm;
	VectorScale(3, &rbuff[3], &rbuff[3], renorm);
	VectorAdd(3, &rmat[3], &rbuff[3], &rbuff[3]);
	// rescale row3
	norm = VectorPower(3, &rbuff[6]);
	renorm = RMAX15 - norm;
	VectorScale(3, &rbuff[6], &rbuff[6], renorm);
	VectorAdd(3, &rmat[6], &rbuff[6], &rbuff[6]);
}

extern int16_t accelOn ;
extern int16_t launched ;
extern int16_t launch_count ;

#define LAUNCH_ACCELERATION ( 2.0 ) // times gravity
#define LAUNCH_VELOCITY ( 10.0 ) // miles per hour
#define LAUNCH_ACCELERATION_BINARY (( int16_t) ( GRAVITY*LAUNCH_ACCELERATION ))
#define EARTH_GRAVITY ( 9.81 ) // meters per second per second
#define FRAME_RATE ( 40.0 ) // computations are done 40 times per second
#define METERSPERSECONDPERMPH ( 4.0/9.0 ) // conversion from MPH to meters/second
#define LAUNCH_VELOCITY_BINARY ( ( int32_t ) ( LAUNCH_VELOCITY*GRAVITY*FRAME_RATE*METERSPERSECONDPERMPH/ EARTH_GRAVITY ) )
#define LAUNCH_DETECT_COUNT ( 20 )

static void roll_pitch_drift(void)
{
	uint16_t gplaneMagnitude  ;
	uint16_t acceleration ;	
	gplaneMagnitude = vector3_mag( gplane[0] , gplane[1] , gplane[2]   ) ;
	acceleration = abs ( gplaneMagnitude - GRAVITY ) ;
	if ( acceleration < ( GRAVITY ))  // thrust must be at least 2 times gravity
	{
		if ( launch_count > 0 )
		{
			launch_count -- ;
		}
	}
	else
	{
		if ( launch_count < LAUNCH_DETECT_COUNT )
		{
			launch_count ++ ;
		}
	}

	if ( launch_count == LAUNCH_DETECT_COUNT )
	{
		launched = 1 ;
	}

	if (  ( acceleration < ( GRAVITY/4 ))&& (launched == 0 ) )

	{
		accelOn = 1 ;
		int16_t gplane_nomalized[3] ;
		vector3_normalize( gplane_nomalized , gplane ) ;
		VectorCross(errorRP, gplane_nomalized, &rmat[6]);
		dirOverGndHrmat[0] = rmat[0] ;
		dirOverGndHrmat[1] = rmat[3] ;
		dirOverGndHrmat[2] = 0 ;
		dirOverGndHGPS[0] = RMAX ;
		dirOverGndHGPS[1] = 0 ;
		dirOverGndHGPS[2] = 0 ;
		if ( rmat[0] > 0 ) // less than 90 degree roll
		{
			VectorCross(errorYawground, dirOverGndHrmat , dirOverGndHGPS );
		}
		else
		{
			errorYawground[0] = 0 ;
			errorYawground[1] = 0 ;
			if ( rmat[3] > 0 )
			{
				errorYawground[2] = -RMAX ;
			}
			else
			{
				errorYawground[2] = RMAX ;
			}
		}
			// convert to plane frame:
			// *** Note: this accomplishes multiplication rmat transpose times errorYawground!!
		MatrixMultiply(1, 3, 3, errorYawplane, errorYawground, rmat) ;
	}
	else
	{
		accelOn = 0 ;
		errorRP[0] = 0 ;
		errorRP[1] = 0 ;
		errorRP[2] = 0 ;
		errorYawplane[0] = 0 ;
		errorYawplane[1] = 0 ;
		errorYawplane[2] = 0 ;
	}

}

#define MAXIMUM_SPIN_DCM_INTEGRAL 20.0 // degrees per second

static void PI_feedback(void)
{
	fractional errorRPScaled[3];
	int16_t kpyaw;
	int16_t kprollpitch;

	// boost the KPs at high spin rate, to compensate for increased error due to calibration error
	// above 50 degrees/second, scale by rotation rate divided by 50

	if (spin_rate < ((uint16_t)(50.0 * DEGPERSEC)))
	{
		kpyaw = KPYAW;
		kprollpitch = KPROLLPITCH;
	}
	else if (spin_rate < ((uint16_t)(500.0 * DEGPERSEC)))
	{
		kpyaw = ((uint16_t)((KPYAW * 8.0) / (50.0 * DEGPERSEC))) * (spin_rate >> 3);
		kprollpitch = ((uint16_t)((KPROLLPITCH * 8.0) / (50.0 * DEGPERSEC))) * (spin_rate >> 3);
	}
	else
	{
		kpyaw = (int16_t)(10.0 * KPYAW);
		kprollpitch = (int16_t)(10.0 * KPROLLPITCH);
	}
	VectorScale(3, omegacorrP, errorYawplane, kpyaw);   // Scale gain = 2
	VectorScale(3, errorRPScaled, errorRP, kprollpitch);// Scale gain = 2
	VectorAdd(3, omegacorrP, omegacorrP, errorRPScaled);

	// turn off the offset integrator while spinning, it doesn't work in that case,
	// and it only causes trouble.

	if (spin_rate < ((uint16_t) (MAXIMUM_SPIN_DCM_INTEGRAL * DEGPERSEC)))
	{	
		gyroCorrectionIntegral[0].WW += (__builtin_mulsu(errorRP[0], KIROLLPITCH)>>3);
		gyroCorrectionIntegral[1].WW += (__builtin_mulsu(errorRP[1], KIROLLPITCH)>>3);
		gyroCorrectionIntegral[2].WW += (__builtin_mulsu(errorRP[2], KIROLLPITCH)>>3);

		gyroCorrectionIntegral[0].WW += (__builtin_mulsu(errorYawplane[0], KIYAW)>>3);
		gyroCorrectionIntegral[1].WW += (__builtin_mulsu(errorYawplane[1], KIYAW)>>3);
		gyroCorrectionIntegral[2].WW += (__builtin_mulsu(errorYawplane[2], KIYAW)>>3);
	}

	omegacorrI[0] = gyroCorrectionIntegral[0]._.W1>>3;
	omegacorrI[1] = gyroCorrectionIntegral[1]._.W1>>3;
	omegacorrI[2] = gyroCorrectionIntegral[2]._.W1>>3;
}



void output_matrix(void)
{
	// This routine makes the direction cosine matrix evident by setting 
	// the three servos to the three values in the matrix.
	union longww accum;
	accum.WW = __builtin_mulss(rmat[6], 4000);
	udb_pwOut[1] = 3000 + accum._.W1;
	accum.WW = __builtin_mulss(rmat[7], 4000);
	udb_pwOut[2] = 3000 + accum._.W1;
	accum.WW = __builtin_mulss(rmat[3], 4000);
	udb_pwOut[3] = 3000 + accum._.W1;
	udb_pwOut[4] = 3000 ;
	udb_pwOut[5] = 3000 ;
	udb_pwOut[6] = 3000 ;
	udb_pwOut[7] = 3000 ;
	udb_pwOut[8] = 3000 ;
}


void dcm_run_imu_step(void)
{
	// update the matrix, renormalize it, adjust for roll and
	// pitch drift, and send it to the servos.
	udb_callback_read_sensors() ;
#if ( GYRO_RANGE == 500 )
	rupdate();                  // local
#elif ( GYRO_RANGE == 1000 )
	rupdate();                  // local
	rupdate();                  // local
#else
#error set GYRO_RANGE to 500 or 1000 in options.h
#endif // GYRO_RANGE
	normalize();                // local
	roll_pitch_drift();         // local
	PI_feedback();              // local
	output_matrix();
}
