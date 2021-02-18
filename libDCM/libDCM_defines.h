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


#ifndef DCM_DEFINES_H
#define DCM_DEFINES_H
#include "dcmTypes.h"

struct dcm_flag_bits {
	uint16_t unused                 : 4;
	uint16_t rollpitch_req          : 1;
	uint16_t gps_history_valid      : 1;
	uint16_t dead_reckon_enable     : 1;
	uint16_t reckon_req             : 1;
	uint16_t first_mag_reading      : 1;
	uint16_t mag_drift_req          : 1;
	uint16_t yaw_req                : 1;
	uint16_t skip_yaw_drift         : 1;
	uint16_t nav_capable            : 1;
	uint16_t nmea_passthrough       : 1; // only used by ublox
	uint16_t init_finished          : 1;
	uint16_t calib_finished         : 1;
};

#define LONGDEG_2_BYTECIR   305 // = (256/360)*((256)**4)/(10**7)
#define COURSEDEG_2_BYTECIR 466 // = (256/360)*((256)**2)/(10**2)

#define RADPERSEC ((int64_t)5632.0/SCALEGYRO)
// one radian per second, in AtoD/2 units

#define DEGPERSEC ((int64_t)98.3/SCALEGYRO)
// one degree per second, in AtoD/2 units

#define GRAVITYM ((int64_t)980.0) 
// 100 times gravity, meters/sec/sec

#define ACCELSCALE ((int32_t)(GRAVITY/GRAVITYM))

#endif // DCM_DEFINES_H
