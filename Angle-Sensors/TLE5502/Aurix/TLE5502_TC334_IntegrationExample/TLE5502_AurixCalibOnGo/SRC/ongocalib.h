/*
 *****************************************************************************
 * Copyright (C) 2025 Infineon Technologies AG. All rights reserved.
 *
 * Infineon Technologies AG (INFINEON) is supplying this file for use
 * exclusively with Infineon's products. This file can be freely
 * distributed within development tools and software supporting such microcontroller
 * products.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT, INCIDENTAL,
 * ASPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************
 */

/* ongocalib.h */

#ifndef ONGOCALIB_H
#define ONGOCALIB_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define M_PI                        3.14159
#define RAD2DEGFACTOR 				(180.00 / M_PI)
#define DEG2RADFACTOR 				(M_PI / 180.00)
#define ANGLE_TOLERANCE 			0.25	  											/*!<  tolerance angle used for calibration process. */
#define ANGLE_TOLERANCE_RAD 		(ANGLE_TOLERANCE * DEG2RADFACTOR)					/*!<  tolerance angle in radians used for the calibration process. */
#define ANGLE180_RAD 				(180.00 * DEG2RADFACTOR)							/*!<  angle position in radians used for the calibration process. */

#define ANGLE45 					45.00 													/*!<  angle position used for the calibration process. */
#define ANGLE135					135.00					 								/*!<  angle position used for the calibration process. */
#define ANGLE225					-135.00					                			/*!<  angle position used for the calibration process. */
#define ANGLE315					-45.00					                				/*!<  angle position used for the calibration process. */

#define ANGLE45_H 					((ANGLE45 + ANGLE_TOLERANCE) * DEG2RADFACTOR)		/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE45_L 					((ANGLE45 - ANGLE_TOLERANCE) * DEG2RADFACTOR)     	/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE135_H 					((ANGLE135 + ANGLE_TOLERANCE) * DEG2RADFACTOR)   	/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE135_L 					((ANGLE135 - ANGLE_TOLERANCE) * DEG2RADFACTOR)   	/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE225_H 					((ANGLE225 + ANGLE_TOLERANCE) * DEG2RADFACTOR)   	/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE225_L 					((ANGLE225 - ANGLE_TOLERANCE) * DEG2RADFACTOR)   	/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE315_H 					((ANGLE315 + ANGLE_TOLERANCE) * DEG2RADFACTOR)   	/*!<  angle position limit in radians used for the calibration process. */
#define ANGLE315_L 					((ANGLE315 - ANGLE_TOLERANCE) * DEG2RADFACTOR)   	/*!<  angle position limit in radians used for the calibration process. */




#define ROTATION_VALID				1u

typedef double float64_t;

typedef struct ONGO_ANALOG_SENSOR_DATA {
	uint32_t sinP_LSB; /*!<  positive sine signal. */
	uint32_t cosP_LSB; /*!<  positive cosine signal. */
	uint32_t sinN_LSB; /*!<  negative sine signal. */
	uint32_t cosN_LSB; /*!<  negative cosine signal. */

	int32_t diff_X; /*!<  differential value for x-channel (cosine). */
	int32_t diff_Y; /*!<  differential value for y-channel (sine). */

	float64_t uncalibratedAngle; /*!<  differential angle. */

} ONGO_ANALOG_SENSOR_DATA_t;

typedef struct ONGO_ROTATION_DATA_STORE {
	bool angle45found;
	bool angle135found;
	uint32_t nr_valid_rotations;
	//bool angle225found;
	//bool angle315found;

	float64_t maxCosN; /*!< max of COSN signal. */ //not anymore differential
	float64_t minCosN; /*!< min of COSN differential signal. */
	float64_t maxSinN; /*!< max of SINN differential signal. */
	float64_t minSinN; /*!< min of SINN differential signal. */

	float64_t maxCosP; /*!< max of COSP differential signal. */
	float64_t minCosP; /*!< min of COSP differential signal. */
	float64_t maxSinP; /*!< max of SINP differential signal. */
	float64_t minSinP; /*!< min of SINP differential signal. */

	float64_t X45; /*!< COS value at 45 deg. */
	float64_t X135; /*!< COS value at 135 deg. */

	float64_t Y45; /*!< SIN value at 45 deg. */
	float64_t Y135; /*!< SIN value at 135 deg. */

	float64_t X45_corr; /*!< COS amplitude and offset corrected value at 45 deg. */
	float64_t X135_corr; /*!< COS amplitude and offset corrected value at 135 deg. */

	float64_t Y45_corr; /*!< SIN amplitude and offset corrected value at 45 deg. */
	float64_t Y135_corr; /*!< SIN amplitude and offset corrected value at 135 deg. */

	float64_t amplitudeXN; /*!< amplitude of COSN signal. */
	float64_t amplitudeYN; /*!< amplitude of SINN signal. */
	float64_t amplitudeXP; /*!< amplitude of COSP signal. */
	float64_t amplitudeYP; /*!< amplitude of SINP signal. */

	float64_t amplitudeX; /*!< amplitude of COS signal. */
	float64_t amplitudeY; /*!< amplitude of SIN signal. */

	float64_t offsetXN; /*!< offset of COSN signal. */
	float64_t offsetYN; /*!< offset of SINN signal. */
	float64_t offsetXP; /*!< offset of COSP signal. */
	float64_t offsetYP; /*!< offset of SINN signal. */

	float64_t offsetX; /*!< offset of COSP signal. */
	float64_t offsetY; /*!< offset of COSN signal. */

	float64_t magnitude45; /*!< M45 value of signal. */
	float64_t magnitude135; /*!< M135 value of signal. */

	float64_t ortho_one_dir; /*!< orthogonality error of signal. */

} ONGO_ROTATION_DATA_STORE_t;

typedef struct ONGO_CALIB_DATA {
	ONGO_ROTATION_DATA_STORE_t rotation_data;

	float64_t sin_ortho; /*!< mean channel correction of orthogonality error  sin (fi). */
	float64_t cos_ortho; /*!< mean channel correction of orthogonality error  cos (fi). */

} ONGO_CALIB_DATA_t;

void ONGO_InitSensorData(uint32_t sinP_in_LSB, uint32_t cosP_in_LSB, uint32_t sinN_in_LSB, uint32_t cosN_in_LSB);
void ONGO_InitCalibData(ONGO_CALIB_DATA_t *calib_param, int32_t max_diff_sin_LSB, int32_t max_diff_cos_LSB,
		int32_t min_diff_sin_LSB, int32_t min_diff_cos_LSB, float64_t sin_45, float64_t cos_45, float64_t sin_135,
		float64_t cos_135);
void ONGO_InitCalibData2(ONGO_CALIB_DATA_t *calib_param, int32_t max_SinP_LSB, int32_t max_CosP_LSB,
		int32_t max_SinN_LSB, int32_t max_CosN_LSB, int32_t min_SinP_LSB, int32_t min_CosP_LSB, int32_t min_SinN_LSB,
		int32_t min_CosN_LSB, float64_t sin_45, float64_t cos_45, float64_t sin_135, float64_t cos_135);
void ONGO_InitCalibData3(ONGO_CALIB_DATA_t *calib_param);
float64_t ONGO_GetUncalibAngle(void);
float64_t ONGO_GetCalibAngle(const ONGO_CALIB_DATA_t *calib_param);
void ONGO_CalibrationFindParam(ONGO_CALIB_DATA_t *param_store);


// External safety mechanisms



/*
Check whether the difference of the calculated angle signals αN and αP is inside
the range of [-αthreshold, αthreshold]

Returns: false for no pass; true for fail
 */
bool SME1_angleComparison_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param);
bool SME2_1_vectorLength_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param);
bool SME2_2_vectorLengthCheck_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param);
bool SME2_3_vectorLengthCheck_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param);
bool SME3_commonModeCheck(const ONGO_CALIB_DATA_t *calib_param);



/*=========================================================================*/

#endif /* ONGOCALIB_H */
