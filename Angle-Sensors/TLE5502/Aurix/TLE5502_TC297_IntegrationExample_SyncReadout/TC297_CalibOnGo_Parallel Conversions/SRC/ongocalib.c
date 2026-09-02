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

/*! \mainpage Infineon OnGoing Calibration Library
 * \version 1.0.2
 * \section intro_sec Introduction
 *
 * This library is intended to help out hardware and software engineers, system integrators and developers with the process of calibrating analog angle sensors.
 * An analog angle sensor does not provide any angle value, only analog sine and cosine (positive and negative) output signals. The angle has to be calculated externally.
 *
 * Analog angle sensors have to be calibrated before they are used in order to achieve the specified angle accuracy.
 * This calibration process includes a compensation of offset, amplitude and non-orthogonality of the output channels in use with the help of an external microcontroller.
 *
 * Calibration is done in a continuous matter (dynamic calibration).
 * Calibration parameters should be first initialized. New parameters will be found after 1-2 turns.
 * The library is intended to be used in a one direction spin.
 *
 * \copyright
 * Copyright (C) 2025 Infineon Technologies AG. All rights reserved.
 *
 * \attention
 * Infineon Technologies AG (INFINEON) is supplying this file for use
 * exclusively with Infineon's products. This file can be freely
 * distributed within development tools and software supporting such microcontroller
 * products.
 * \n
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT, INCIDENTAL,
 * ASPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *	\warning
 * It is in the responsibility of the system integrator to insure that:
 * 	- the magnet is large enough to ensure that the non-homogeneity of the magnetic field in the sensing area is negligible.
 *	\warning
 * 	- the mechanical tolerances of the sensor-magnet assembly are minimal.
 *	\warning
 * 	- the output amplitude together with the resolution of the ADC in use matches the requirement in angle accuracy.
 *	\warning
 * 	- the calibration parameters are initialized with correct values.
 *	\warning
 *	- when new calibration parameters have been found, the library will do the mathematical calculations
 *
 * \see https://www.infineon.com/dgdl/Infineon-TLE5xxx(D)_Calibration_360_AN-v02_00-AN-v02_00-EN.pdf?fileId=5546d46264a8de7e0164f09d8bfa228d
 * \this example has been implemented by using the AURIX-lite-Kit TCxx4. Please be aware that ADC configuration will be different if another MCU is used.
 *
 * \section usage_sec Usage
 *
 * \note This library assumes that you have connected the analog angle sensor to an ADC and you can easily get the SINP, COSP, SINN, COSN digitized values.
 *
 * The one time calibration process requires you to:
 * 	- turn the magnetic field in one direction while acquiring SIN & COS data
 * 	- afterward the firmware library will compute:
 * 		- new amplitude & offset correction values
 *
 * If your application is subjected to temperature variations you should perform dynamic/continuous calibration as the one time calibration can help
 * reduce only the temperature independent offset.
 *
 * \note  Because the calibration process is quite intensive computationally the system integrator should use either a microcontroller with an FPU or a dedicated math coprocessor/hardware CORDIC.
 *
 *
 * \section example_sec Example
 *
 * \dot
 *  digraph usage_graph
 *  {
 *  	fontname="Helvetica";
 *  	init [label = "ONGO_InitCalibData"];
 *  	readout [label = "Your periodic readout function for SIN & COS signals", shape=box, style=filled, color=".7 .3 1.0"];
 *  	init_sensor_data [label = "ONGO_InitSensorData"];
 *  	uncalib_angle_get [label = "ONGO_GetUncalibAngle"];
 *  	calib_find_param [label = "ONGO_CalibrationFindParam"];
 *		angle_get [label = "ONGO_GetCalibAngle"];
 *
 *  	init -> readout [weight=8];
 *  	readout -> init_sensor_data [color = red];
 *  	init_sensor_data -> calib_find_param [color = red];
 *  	init_sensor_data -> uncalib_angle_get [style = dotted];
 *  	calib_find_param -> angle_get [color = red];
 *  	angle_get -> readout [color = red];
 * }
 * \enddot
 *
 * Code Example 1:
 *
 * \code{.c}
 * ONGO_CALIB_DATA_t calibration_store_params;
 *
 * float angle_read = 0;
 * uint32_t sinP_in_LSB = 0;
 * uint32_t cosP_in_LSB = 0;
 * uint32_t sinN_in_LSB = 0;
 * uint32_t cosN_in_LSB = 0;
 *
 * int main(void)
 * {
 *  	ONGO_InitCalibData(&calibration_store_params, max_diff_sin_LSB, max_diff_cos_LSB,
 *  	 min_diff_sin_LSB, min_diff_cos_LSB, sin_45, cos_45, sin_135, cos_135);
 *
 *		while(1U)
 *		{
 * 			// read sensor data
 *			// AnalogRead(ch) is a user defined function to read the analog voltage on P and N channels
 *			sinP_in_LSB = AnalogRead(ch0);
 *			cosP_in_LSB = AnalogRead(ch1);
 *			sinN_in_LSB = AnalogRead(ch2);
 *			cosN_in_LSB = AnalogRead(ch3);
 *			ONGO_InitSensorData(sinP_in_LSB, cosP_in_LSB, sinN_in_LSB, cosN_in_LSB);
 *			ONGO_CalibrationFindParam(&calibration_store_params);
 *			angle_read = ONGO_GetCalibAngle(&calibration_store_params);
 *		}
 * }
 * \endcode
 *
 * \section consider_sec Other considerations
 *
 * \see https://www.infineon.com/dgdl/Infineon-TLE5xxx(D)_Calibration_360_AN-v02_00-AN-v02_00-EN.pdf?fileId=5546d46264a8de7e0164f09d8bfa228d
 */

#include "ongocalib.h"
static ONGO_ANALOG_SENSOR_DATA_t sensor_data;

// Sensor variant dependent defines. Current values are only usable with TLE5501
#define SME_VAL1  4.2
#define SME_VAL2_LOW  0.76
#define SME_VAL2_HIGH  1.24
#define SME_VAL3_VCM_POS  0.08  // [0.955·VDD , 1.045·VDD]
#define SME_VAL3_VCM_NEG  -0.08 // [0.955·VDD , 1.045·VDD]


void ONGO_CalculateDiff(void);
float ONGO_ONGODiffCalculate(const ONGO_CALIB_DATA_t *calib_param);
void ONGO_CalibOneDirectionMinMax(ONGO_ROTATION_DATA_STORE_t* rotation_data_store);
void ONGO_CalibFindMinMax(ONGO_ROTATION_DATA_STORE_t *rotation_data_store);
void ONGO_CalibCalculateOneDirection(ONGO_ROTATION_DATA_STORE_t *rotation_data_store);
void ONGO_CalibCalculateOrtho(ONGO_CALIB_DATA_t *calib_param);

/**
 * @brief
 * Function used for ONGO_ANALOG_SENSOR_DATA_t data assignment.
 * @details
 * This function must be called after the data readout.
 * @details
 * The initialized pointer will be used in the angle calculation and calibration process.
 * @param[in]
 * cosP_in_LSB is the sensor value in LSB corresponding to the positive cosinus.
 * @param[in]
 * cosN_in_LSB is the sensor value in LSB corresponding to the negative cosinus.
 * @param[in]
 * sinP_in_LSB is the sensor value in LSB corresponding to the positive sinus.
 * @param[in]
 * sinN_in_LSB is the sensor value in LSB corresponding to the negative sinus.
 * @return
 * Nothing
 */
void ONGO_InitSensorData(uint32_t sinP_in_LSB, uint32_t cosP_in_LSB, uint32_t sinN_in_LSB, uint32_t cosN_in_LSB) {
    sensor_data.cosP_LSB = cosP_in_LSB;
    sensor_data.cosN_LSB = cosN_in_LSB;
    sensor_data.sinP_LSB = sinP_in_LSB;
    sensor_data.sinN_LSB = sinN_in_LSB;
    ONGO_CalculateDiff();
}

/**
 * @brief
 * Function used for ONGO_CALIB_DATA_t data initialization.
 * The pointer of type ONGO_CALIB_DATA_t will be used to store calibration parameters.
 * @details
 * This function must be called once before ONGO_CalibrationFindParam(..).
 * Use this or ONGO_InitCalibData2(..).
 * @param[in]
 * calib_param is a pointer of type ONGO_CALIB_DATA_t.
 * @param[in]
 * max_diff_sin_LSB is the maximum differential sin signal in LSB.
 * @param[in]
 * max_diff_cos_LSB is the maximum differential cos signal in LSB.
 * @param[in]
 * min_diff_sin_LSB is the minimum differential sin signal in LSB.
 * @param[in]
 * min_diff_cos_LSB is the minimum differential cos signal in LSB.
 * @param[in]
 * sin_45 is the differential sin component in LSB at a 45 deg position.
 * @param[in]
 * cos_45 is the differential cos component in LSB at a 45 deg position.
 * @param[in]
 * sin_135 is the differential sin component in LSB at a 135 deg position.
 * @param[in]
 * cos_135 is the differential cos component in LSB at a 135 deg position.
 * @return
 * Nothing
 */
void ONGO_InitCalibData(ONGO_CALIB_DATA_t *calib_param, int32_t max_diff_sin_LSB, int32_t max_diff_cos_LSB, int32_t min_diff_sin_LSB,
        int32_t min_diff_cos_LSB, float sin_45, float cos_45, float sin_135, float cos_135) {

    calib_param->rotation_data.angle135found = false;
    calib_param->rotation_data.angle45found = false;
    calib_param->rotation_data.nr_valid_rotations = 0u;

    calib_param->rotation_data.maxCosN = max_diff_cos_LSB;
    calib_param->rotation_data.minCosN = min_diff_cos_LSB;
    calib_param->rotation_data.maxSinN = max_diff_sin_LSB;
    calib_param->rotation_data.minSinN = min_diff_sin_LSB;

    calib_param->rotation_data.maxCosP = max_diff_cos_LSB;
    calib_param->rotation_data.minCosP = min_diff_cos_LSB;
    calib_param->rotation_data.maxSinP = max_diff_sin_LSB;
    calib_param->rotation_data.minSinP = min_diff_sin_LSB;

    calib_param->rotation_data.X45 = cos_45;
    calib_param->rotation_data.X135 = cos_135;

    calib_param->rotation_data.Y45 = sin_45;
    calib_param->rotation_data.Y135 = sin_135;

    ONGO_CalibCalculateOneDirection(&(calib_param->rotation_data));
    ONGO_CalibCalculateOrtho(calib_param);
}

/**
 * @brief
 * Function used for ONGO_CALIB_DATA_t data initialization.
 * The pointer of type ONGO_CALIB_DATA_t will be used to store calibration parameters.
 * @details
 * This function must be called once before ONGO_CalibrationFindParam(..).
 * Use this or ONGO_InitCalibData(..).
 * @param[in]
 * calib_param is a pointer of type ONGO_CALIB_DATA_t.
 * @param[in]
 * max_SinP_LSB is the maximum P channel sin signal in LSB.
 * @param[in]
 * max_CosP_LSB is the maximum P channel cos signal in LSB.
 * @param[in]
 * max_SinN_LSB is the maximum N channel sin signal in LSB.
 * @param[in]
 * max_CosN_LSB is the maximum N channel cos signal in LSB.
 * @param[in]
 * min_SinP_LSB is the minimum P channel sin signal in LSB.
 * @param[in]
 * min_CosP_LSB is the minimum P channel cos signal in LSB.
 * @param[in]
 * min_SinN_LSB is the minimum N channel sin signal in LSB.
 * @param[in]
 * min_CosN_LSB is the minimum N channel cos signal in LSB.
 * @param[in]
 * sin_45 is the differential sin component in LSB at a 45 deg position.
 * @param[in]
 * cos_45 is the differential cos component in LSB at a 45 deg position.
 * @param[in]
 * sin_135 is the differential sin component in LSB at a 135 deg position.
 * @param[in]
 * cos_135 is the differential cos component in LSB at a 135 deg position.
 * @return
 * Nothing
 */
void ONGO_InitCalibData2(ONGO_CALIB_DATA_t *calib_param, int32_t max_SinP_LSB, int32_t max_CosP_LSB, int32_t max_SinN_LSB,
        int32_t max_CosN_LSB, int32_t min_SinP_LSB, int32_t min_CosP_LSB, int32_t min_SinN_LSB, int32_t min_CosN_LSB, float sin_45,
        float cos_45, float sin_135, float cos_135) {

    calib_param->rotation_data.angle135found = false;
    calib_param->rotation_data.angle45found = false;
    calib_param->rotation_data.nr_valid_rotations = 0u;

    calib_param->rotation_data.maxCosN = (float)(max_CosP_LSB - min_CosN_LSB);
    calib_param->rotation_data.minCosN = (float)(min_CosP_LSB - max_CosN_LSB);
    calib_param->rotation_data.maxSinN = (float)(max_SinP_LSB - min_SinN_LSB);
    calib_param->rotation_data.minSinN = (float)(min_SinP_LSB - max_SinN_LSB);

    calib_param->rotation_data.maxCosP = (float)(max_CosP_LSB - min_CosN_LSB);
    calib_param->rotation_data.minCosP = (float)(min_CosP_LSB - max_CosN_LSB);
    calib_param->rotation_data.maxSinP = (float)(max_SinP_LSB - min_SinN_LSB);
    calib_param->rotation_data.minSinP = (float)(min_SinP_LSB - max_SinN_LSB);

    calib_param->rotation_data.X45 = cos_45;
    calib_param->rotation_data.X135 = cos_135;

    calib_param->rotation_data.Y45 = sin_45;
    calib_param->rotation_data.Y135 = sin_135;

    ONGO_CalibCalculateOneDirection(&(calib_param->rotation_data));
    ONGO_CalibCalculateOrtho(calib_param);
}

void ONGO_InitCalibData3(ONGO_CALIB_DATA_t *calib_param)
{
    calib_param->rotation_data.angle135found = false;
    calib_param->rotation_data.angle45found = false;
    calib_param->rotation_data.nr_valid_rotations = 0u;

    calib_param->rotation_data.maxCosN = 0;
    calib_param->rotation_data.minCosN = 4095;
    calib_param->rotation_data.maxSinN = 0;
    calib_param->rotation_data.minSinN = 4095;

    calib_param->rotation_data.maxCosN = 0;
    calib_param->rotation_data.minCosN = 4095;
    calib_param->rotation_data.maxSinN = 0;
    calib_param->rotation_data.minSinN = 4095;

    ONGO_CalibCalculateOneDirection(&(calib_param->rotation_data));
    ONGO_CalibCalculateOrtho(calib_param);

}
/**
 * @brief
 * The function will calculate the differential values of sin,cos signal and angle.
 * @details
 * The differential values will be used to get the uncalibrated angle value or in the calibration procedure.
 * It's called internally by ONGO_InitSensorData().
 * @return
 * Nothing
 */
void ONGO_CalculateDiff() {
    sensor_data.diff_Y = (int32_t) sensor_data.sinP_LSB - (int32_t) sensor_data.sinN_LSB;
    sensor_data.diff_X = (int32_t) sensor_data.cosP_LSB - (int32_t) sensor_data.cosN_LSB;
    sensor_data.uncalibratedAngle = atan2((float) sensor_data.diff_Y, (float) sensor_data.diff_X);
}

/**
 * @brief
 * Calculate the calibrated angle value.
 * @details
 * It uses the stored calibration parameters to calculate the calibrated angle value.
 * The function is called internally by ONGO_GetCalibAngle(..).
 * @param[in]
 * calib_param is a pointer of type ONGO_CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Returns the calibrated angle value in radians.
 */
float ONGO_ONGODiffCalculate(const ONGO_CALIB_DATA_t *calib_param) {
    float corr_X = ((float)sensor_data.diff_X - calib_param->rotation_data.offsetX) / calib_param->rotation_data.amplitudeX;
    float corr_Y = ((float)sensor_data.diff_Y - calib_param->rotation_data.offsetY) / calib_param->rotation_data.amplitudeY;

    float ortho_Y = (corr_Y - (corr_X * calib_param->sin_ortho)) / calib_param->cos_ortho;

    //return (atan2(ortho_Y, corr_X) + ANGLE180_RAD);
    return (atan2(ortho_Y, corr_X));
}

/**
 * @brief
 * Calculates the uncalibrated angle.
 * @details
 * This function uses differential sensor data to extract the angle.
 * @return
 * Returns the uncalibrated angle in degrees.
 */
float ONGO_GetUncalibAngle(void) {
    return (sensor_data.uncalibratedAngle) * RAD2DEGFACTOR;
}

/**
 * @brief
 * Calculates the calibrated angle.
 * @param[in]
 * calib_param is a pointer of type ONGO_CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Returns the calibrated angle in degrees.
 */
float ONGO_GetCalibAngle(const ONGO_CALIB_DATA_t *calib_param) {
    return (ONGO_ONGODiffCalculate(calib_param)) * RAD2DEGFACTOR;
}

/**
 * @brief
 * This function is used to find new calibration parameters.
 * @details
 * When new calibration parameters have been found, this function will do mathematical calculations, which will take more time for one moment.
 * This function will extract the maximum, minimum voltage levels, amplitude, offset, and orthogonality.
 * Based on the found values, it will calculate the necessary calibration parameters.
 * @param[in]
 * calib_param is a pointer of type ONGO_CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Nothing
 */
void ONGO_CalibrationFindParam(ONGO_CALIB_DATA_t *calib_param) {
    ONGO_CalibOneDirectionMinMax(&calib_param->rotation_data);

    if (calib_param->rotation_data.angle135found && calib_param->rotation_data.angle45found)
    {
        calib_param->rotation_data.nr_valid_rotations++;
        calib_param->rotation_data.angle45found = false;
        calib_param->rotation_data.angle135found = false;
    }

    if (calib_param->rotation_data.nr_valid_rotations > ROTATION_VALID)
    {
        calib_param->rotation_data.nr_valid_rotations ^= calib_param->rotation_data.nr_valid_rotations;
        ONGO_CalibCalculateOneDirection(&calib_param->rotation_data);
        ONGO_CalibCalculateOrtho(calib_param);
    }
}

/**
 * @brief
 * Finds and Stores ONGO_ROTATION_DATA_STORE_t parameters.
 * @details
 * This function is called internally by ONGO_CalibrationFindParam().
 * Used to search for the magnitude components (at 45 and 135 deg) and min/max signal level.
 * @param[in]
 * rotation_data_store is a pointer of type ONGO_ROTATION_DATA_STORE_t, this structure holds the current rotation parameters.
 * @return
 * Nothing
 */
void ONGO_CalibOneDirectionMinMax(ONGO_ROTATION_DATA_STORE_t* rotation_data_store) {
    ONGO_CalibFindMinMax(rotation_data_store);

    if ((sensor_data.uncalibratedAngle <= ANGLE45_H) && (sensor_data.uncalibratedAngle >= ANGLE45_L))
    {
        rotation_data_store->X45 = (float) sensor_data.diff_X;
        rotation_data_store->Y45 = (float) sensor_data.diff_Y;
        rotation_data_store->angle45found = true;
    } else if ((sensor_data.uncalibratedAngle <= ANGLE135_H) && (sensor_data.uncalibratedAngle >= ANGLE135_L))
    {
        rotation_data_store->X135 = (float) sensor_data.diff_X;
        rotation_data_store->Y135 = (float) sensor_data.diff_Y;
        rotation_data_store->angle135found = true;
    }else{

    }

}

/**
 * @brief
 * Finds and Stores ONGO_ROTATION_DATA_STORE_t parameters.
 * @details
 * Used to search for the min/max differential signal level.
 * This function is called internally by ONGO_CalibOneDirectionMinMax().
 * @param[in]
 * rotation_data_store is a pointer of type ONGO_ROTATION_DATA_STORE_t, this structure holds the current rotation parameters.
 * @return
 * Nothing
 */
void ONGO_CalibFindMinMax(ONGO_ROTATION_DATA_STORE_t *rotation_data_store) {

    if (sensor_data.cosN_LSB < rotation_data_store->minCosN && sensor_data.cosN_LSB!=0)//(sensor_data.diff_X < rotation_data_store->minCos)
     {
         //rotation_data_store->minCos = sensor_data.diff_X;
         rotation_data_store->minCosN = sensor_data.cosN_LSB;
     }
     if (sensor_data.cosN_LSB > rotation_data_store->maxCosN)//(sensor_data.diff_X > rotation_data_store->maxCos)
     {
         //rotation_data_store->maxCos = sensor_data.diff_X;
         rotation_data_store->maxCosN = sensor_data.cosN_LSB;
     }
     if (sensor_data.cosP_LSB < rotation_data_store->minCosP && sensor_data.cosP_LSB!=0)
     {
         rotation_data_store->minCosP = sensor_data.cosP_LSB;
     }
     if (sensor_data.cosP_LSB > rotation_data_store->maxCosP)
     {
         rotation_data_store->maxCosP = sensor_data.cosP_LSB;
     }


     if (sensor_data.sinN_LSB < rotation_data_store->minSinN && sensor_data.sinN_LSB!=0)
     {
         rotation_data_store->minSinN = sensor_data.sinN_LSB;
     }
     if (sensor_data.sinN_LSB > rotation_data_store->maxSinN)
     {
         rotation_data_store->maxSinN = sensor_data.sinN_LSB;
     }
     if (sensor_data.sinP_LSB < rotation_data_store->minSinP && sensor_data.sinP_LSB!=0)
     {
         rotation_data_store->minSinP = sensor_data.sinP_LSB;
     }
     if (sensor_data.sinP_LSB > rotation_data_store->maxSinP)
     {
         rotation_data_store->maxSinP = sensor_data.sinP_LSB;
     }

}

/**
 * @brief
 * Calculates and stores ONGO_ROTATION_DATA_STORE_t parameters, based on found parameters.
 * @details
 * Should be called only if rotation_data_store->calibration_done is true.
 * This function is called internally by ONGO_CalibOneDirectionMinMax().
 * Calculates the amplitude, offset, magnitude and orthogonality for one direction (CW or CCW).
 * @param[in]
 * rotation_data_store is a pointer of type ONGO_ROTATION_DATA_STORE_t, this structure holds the current rotation parameters.
 * @return
 * Nothing
 */
void ONGO_CalibCalculateOneDirection(ONGO_ROTATION_DATA_STORE_t *rotation_data_store) {
    rotation_data_store->amplitudeXN = ((float) rotation_data_store->maxCosN - (float) rotation_data_store->minCosN) / 2.00;
    rotation_data_store->amplitudeYN = ((float) rotation_data_store->maxSinN - (float) rotation_data_store->minSinN) / 2.00;
    rotation_data_store->amplitudeXP = ((float) rotation_data_store->maxCosP - (float) rotation_data_store->minCosP) / 2.00;
    rotation_data_store->amplitudeYP = ((float) rotation_data_store->maxSinP - (float) rotation_data_store->minSinP) / 2.00;

    rotation_data_store->amplitudeY = rotation_data_store->amplitudeYP - rotation_data_store->amplitudeYN;
    rotation_data_store->amplitudeX = rotation_data_store->amplitudeXP - rotation_data_store->amplitudeXN;

    rotation_data_store->offsetXN = ((float) rotation_data_store->maxCosN + (float) rotation_data_store->minCosN) / 2.00;
    rotation_data_store->offsetYN = ((float) rotation_data_store->maxSinN + (float) rotation_data_store->minSinN) / 2.00;
    rotation_data_store->offsetXP = ((float) rotation_data_store->maxCosP + (float) rotation_data_store->minCosP) / 2.00;
    rotation_data_store->offsetYP = ((float) rotation_data_store->maxSinP + (float) rotation_data_store->minSinP) / 2.00;

    rotation_data_store->offsetX = rotation_data_store->offsetYP - rotation_data_store->offsetYN;
    rotation_data_store->offsetY = rotation_data_store->offsetYP - rotation_data_store->offsetYN;

    // reinitialize to find new min/max values
    rotation_data_store->maxCosP = -0x07FFFFFFF;
    rotation_data_store->minCosP = 0x07FFFFFFF;
    rotation_data_store->maxSinP = -0x07FFFFFFF;
    rotation_data_store->minSinP = 0x07FFFFFFF;

    rotation_data_store->maxCosN = -0x07FFFFFFF;
    rotation_data_store->minCosN = 0x07FFFFFFF;
    rotation_data_store->maxSinN = -0x07FFFFFFF;
    rotation_data_store->minSinN = 0x07FFFFFFF;

    rotation_data_store->X45_corr = (rotation_data_store->X45 - rotation_data_store->offsetX) / rotation_data_store->amplitudeX;
    rotation_data_store->X135_corr = (rotation_data_store->X135 - rotation_data_store->offsetX) / rotation_data_store->amplitudeX;

    rotation_data_store->Y45_corr = (rotation_data_store->Y45 - rotation_data_store->offsetY) / rotation_data_store->amplitudeY;
    rotation_data_store->Y135_corr = (rotation_data_store->Y135 - rotation_data_store->offsetY) / rotation_data_store->amplitudeY;

    rotation_data_store->magnitude45 = sqrt(
            (rotation_data_store->X45_corr * rotation_data_store->X45_corr)
            + (rotation_data_store->Y45_corr * rotation_data_store->Y45_corr));
    rotation_data_store->magnitude135 = sqrt(
            (rotation_data_store->X135_corr * rotation_data_store->X135_corr)
            + (rotation_data_store->Y135_corr * rotation_data_store->Y135_corr));

    rotation_data_store->ortho_one_dir = 2.00
            * atan2((rotation_data_store->magnitude135 - rotation_data_store->magnitude45),
                    (rotation_data_store->magnitude135 + rotation_data_store->magnitude45));

}

/**
 * @brief
 * Calculates and stores ONGO_CALIB_DATA_t parameters, based on found parameters from both CW and CCW rotations.
 * @details
 * Should be called only if calib_param->full_calibration_performed is true.
 * This function is called internally by ONGO_CalibrationFindParam(..).
 * Calculates the mean amplitude, offset and orthogonality sin and cos constants, based on found parameters from both CW and CCW rotations.
 * @param[in]
 * calib_param is a pointer of type ONGO_CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Nothing
 */
void ONGO_CalibCalculateOrtho(ONGO_CALIB_DATA_t *calib_param) {

    calib_param->sin_ortho = sin(-(calib_param->rotation_data.ortho_one_dir));
    calib_param->cos_ortho = cos(-(calib_param->rotation_data.ortho_one_dir));
}




/*
Check whether the difference of the calculated angle signals αN and αP is inside
the range of [-αthreshold, αthreshold]

This function uses the stored calibration parameters to calculate the calibrated angle values. These values are then used to check if their difference is in the range of [-threshold, threshold]
This performs the Safety Mechanism External 1

Returns: false for no pass; true for pass
 */
bool SME1_angleComparison_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param)
{
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data.offsetXP) / calib_param->rotation_data.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data.offsetYP) / calib_param->rotation_data.amplitudeYP;
    volatile float CosN_display_calibrated = ( (float)sensor_data.cosN_LSB - calib_param->rotation_data.offsetXN) / calib_param->rotation_data.amplitudeXN;
    volatile float SinN_display_calibrated = ( (float)sensor_data.sinN_LSB - calib_param->rotation_data.offsetYN) / calib_param->rotation_data.amplitudeYN;


    volatile float SinP_ortho_corrected = (SinP_display_calibrated - (CosP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho;
    volatile float SinN_ortho_corrected = (SinN_display_calibrated - (CosN_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho;

    volatile float angle_calibrated_P = atan2l(SinP_ortho_corrected, CosP_display_calibrated) * RAD2DEGFACTOR;
    volatile float angle_calibrated_N = atan2l(SinN_ortho_corrected, CosN_display_calibrated) * RAD2DEGFACTOR;

    angle_calibrated_N = angle_calibrated_N - 180;
    if(angle_calibrated_N < -180)
    {
        angle_calibrated_N += 360;
    }

    if ( fabsl(angle_calibrated_P - angle_calibrated_N)  > SME_VAL1  )
    {
        return false;
    }
    else
    {
        return true;
    }
}


/*
 * Check whether the calculated signal V_P = SQRT((SIN_P)2 + (COS_P)2) and
V_N = SQRT((SIN_N)^2 + (COS_N)^2) are both inside the range of [rmin, rmax]
 */


bool SME2_1_vectorLength_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param)
{
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data.offsetXP) / calib_param->rotation_data.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data.offsetYP) / calib_param->rotation_data.amplitudeYP;
    volatile float CosN_display_calibrated = ( (float)sensor_data.cosN_LSB - calib_param->rotation_data.offsetXN) / calib_param->rotation_data.amplitudeXN;
    volatile float SinN_display_calibrated = ( (float)sensor_data.sinN_LSB - calib_param->rotation_data.offsetYN) / calib_param->rotation_data.amplitudeYN;


    volatile float SinP_ortho_corrected = (float)( (SinP_display_calibrated - (CosP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float CosP_ortho_corrected = (float)( (CosP_display_calibrated - (SinP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float SinN_ortho_corrected = (float)( (SinN_display_calibrated - (CosN_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float CosN_ortho_corrected = (float)( (CosN_display_calibrated - (SinN_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);


    volatile float V_P = (float)sqrt( pow(CosP_ortho_corrected,2) + pow(SinP_ortho_corrected,2)  );
    volatile float V_N = (float)sqrt( pow(CosN_ortho_corrected,2) + pow(SinN_ortho_corrected,2)  );

    if(V_P >= SME_VAL2_LOW && V_P < SME_VAL2_HIGH && V_N >= SME_VAL2_LOW && V_N < SME_VAL2_HIGH )
    {
        return true;
    }
    else
    {
        return false;
    }
}



bool SME2_2_vectorLengthCheck_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param)
{
    volatile double CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data.offsetXP) / calib_param->rotation_data.amplitudeXP;
    volatile double SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data.offsetYP) / calib_param->rotation_data.amplitudeYP;


    volatile double SinP_ortho_corrected = (float)( (SinP_display_calibrated - (CosP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile double CosP_ortho_corrected = (float)( (CosP_display_calibrated - (SinP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);


    volatile double V_P = sqrt( pow(CosP_ortho_corrected,2) + pow(SinP_ortho_corrected,2)  );

    if(V_P >= SME_VAL2_LOW && V_P < SME_VAL2_HIGH)
    {
        return true;
    }
    else
    {
        return false;
    }

}



bool SME2_3_vectorLengthCheck_AutoCalibration(const ONGO_CALIB_DATA_t *calib_param)
{


    //scaled to 5 volts reference and 12 bits resolution
    /* float corr_X = ((float)sensor_data.diff_X - calib_param->rotation_data.offsetX) / calib_param->rotation_data.amplitudeX; //  * 5 /4095
    float corr_Y = ((float)sensor_data.diff_Y - calib_param->rotation_data.offsetY) / calib_param->rotation_data.amplitudeY; //* 5 /4095

    float ortho_Y = (corr_Y - (corr_X * calib_param->sin_ortho)) / calib_param->cos_ortho;

    volatile float V_PD = sqrt( pow((ortho_Y),2) + pow((corr_X),2) );



    if( V_PD >= SME_VAL2_LOW && V_PD < SME_VAL2_HIGH )
    {
        return true;
    }
    else
    {
        return false;
    }
    */
    //scaled to 5 volts reference and 12 bits resolution
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data.offsetXP) / calib_param->rotation_data.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data.offsetYP) / calib_param->rotation_data.amplitudeYP;


    volatile float SinP_ortho_corrected = (float)( (SinP_display_calibrated - (CosP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float CosP_ortho_corrected = (float)( (CosP_display_calibrated - (SinP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);

    volatile float V_PD = sqrt( pow((SinP_ortho_corrected),2) + pow((CosP_ortho_corrected),2) );


    if( V_PD >= SME_VAL2_LOW && V_PD < SME_VAL2_HIGH )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SME3_commonModeCheck(const ONGO_CALIB_DATA_t *calib_param)
{
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data.offsetXP) / calib_param->rotation_data.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data.offsetYP) / calib_param->rotation_data.amplitudeYP;
    volatile float CosN_display_calibrated = ( (float)sensor_data.cosN_LSB - calib_param->rotation_data.offsetXN) / calib_param->rotation_data.amplitudeXN;
    volatile float SinN_display_calibrated = ( (float)sensor_data.sinN_LSB - calib_param->rotation_data.offsetYN) / calib_param->rotation_data.amplitudeYN;

    float Vcm_s = (float)SinP_display_calibrated + (float)SinN_display_calibrated;
    float Vcm_c = (float)CosP_display_calibrated + (float)CosN_display_calibrated;

    if(Vcm_s > SME_VAL3_VCM_NEG && Vcm_s < SME_VAL3_VCM_POS  && Vcm_c > SME_VAL3_VCM_NEG && Vcm_c < SME_VAL3_VCM_POS )
    {
        return true;
    }
    else
    {
        return false;
    }
}

