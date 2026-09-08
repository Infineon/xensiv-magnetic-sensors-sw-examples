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

/*! \mainpage Infineon One Point Calibration Library
 * \version 1.0.2
 * \section intro_sec Introduction
 *
 * This library is intended to help out hardware and software engineers, system integrators and developers with the process of calibrating analog angle sensors.
 * An analog angle sensor does not provide any angle value, only analog sine and cosine (positive and negative) output signals. The angle has to be calculated externally.
 *
 * Analog angle sensors have to be calibrated before they are used in order to achieve the specified angle accuracy.
 * This calibration process includes a compensation of offset, amplitude and non-orthogonality of the output channels in use with the help of an external microcontroller.
 * Calibration can be done in a one time fashion at the start-up of the system for example, or in a continuous matter (dynamic calibration).
 * This library and the examples given talk about the one-time calibration however the functions could be easily extended to support continuous calibration.
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
 * \warning
 * It is in the responsibility of the system integrator to insure that:
 * 	- the magnet is large enough to ensure that the non-homogeneity of the magnetic field in the sensing area is negligible.
 * \warning
 * 	- the mechanical tolerances of the sensor-magnet assembly are minimal.
 * \warning
 * 	- the output amplitude together with the resolution of the ADC in use matches the requirement in angle accuracy.
 *
 * \see https://www.infineon.com/dgdl/Infineon-TLE5xxx(D)_Calibration_360_AN-v02_00-AN-v02_00-EN.pdf?fileId=5546d46264a8de7e0164f09d8bfa228d
 *
 *
 * \section usage_sec Usage
 *
 * \note This library assumes that you have connected the analog angle sensor to an ADC and you can easily get the SINP, COSP, SINN, COSN digitized values.
 *
 * The one time calibration process requires you to:
 * 	- turn the magnetic field 360 deg in one direction while acquiring SIN & COS data
 * 	- turn the magnetif field 360 deg in the opposite direction while aquiring SIN & COS data
 * 	- afterward the firmware library will compute:
 * 		- amplitude & offset correction values for left turn
 * 		- amplitude & offset correction values for right turn
 * 		- mean amplitude & offset values
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
 *  	init [label = "ONECALIB_InitCalibData"];
 *  	readout [label = "Your periodic readout function for SIN & COS signals", shape=box, style=filled, color=".7 .3 1.0"];
 *  	init_sensor_data [label = "ONECALIB_InitSensorData"];
 *  	is_calib_done [label = "Is calibration\n done?",shape = diamond];
 *  	uncalib_angle_get [fontname="Helvetica-Bold", label = "ONECALIB_GetUncalibAngle", fontsize = 9];
 *		is_rotating_cw [label = "Is rotating\n clockwise?",shape = diamond];
 *		calib_cw [label = "ONECALIB_CalibrationFindParam(..,true,false)"];
 *		calib_ccw[label = "ONECALIB_CalibrationFindParam(..,false,true)"];
 *		calib_angle_get [label = "ONECALIB_GetCalibAngle", color = red ];
 *
 *  	init -> readout [style = bold, color = red, weight=8];
 *  	readout -> init_sensor_data [style = bold, color = red, weight=8];
 *  	init_sensor_data -> uncalib_angle_get [style = dotted];
 *  	//uncalib_angle_get -> readout [style = dotted];
 *  	init_sensor_data -> is_calib_done [style = bold, color = red, weight=8];
 *  	is_calib_done -> is_rotating_cw [style = bold, label="NO", weight=8];  // so is this;
 *  	is_rotating_cw -> calib_cw [style = bold, label="YES", weight=4];
 *  	is_rotating_cw -> calib_ccw [style = bold, label="NO", weight=4];
 *  	calib_cw -> readout [weight = 1];
 *  	calib_ccw -> readout [weight = 7];
 *  	is_calib_done-> calib_angle_get [style=bold,label="YES",color = red];
 *  	calib_angle_get -> readout [color = red];
 * }
 * \enddot
 *
 * Code Example 1:
 *
 * \code{.c}
 * CALIB_DATA_t calibration_store_params;
 *
 * float angle_read = 0;
 * uint32_t sinP_in_LSB = 0;
 * uint32_t cosP_in_LSB = 0;
 * uint32_t sinN_in_LSB = 0;
 * uint32_t cosN_in_LSB = 0;
 *
 * int main(void)
 * {
 *  	IfxCpu_enableInterrupts();
 *
 *  // !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
 *  // Enable the watchdogs and service them periodically if it is required
 *  //
 *  IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
 *  IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
 *
 *   // Wait for CPU sync event
 *   IfxCpu_emitEvent(&cpuSyncEvent);
 *   IfxCpu_waitEvent(&cpuSyncEvent, 1);
 *
 *   evadc_g8_init_queue_continuous();
 *
 *
 *   CALIB_DATA_t calibration_store_params;
 *   volatile float angle_read = 0;
 *   uint16 sinP_in_LSB = 0;
 *   uint16 cosP_in_LSB = 0;
 *   uint16 sinN_in_LSB = 0;
 *   uint16 cosN_in_LSB = 0;
 *
 *   ONECALIB_InitCalibData(&calibration_store_params);
 *   int safetyPasses = 0;
 *   int safetyFails = 0;
 *
 *		while(1U)
 *		{
 * 	//------------------- CALIBRATION FOR CW DIRECTION -------------------------------
 * 			while (!calibration_store_params.rotation_data_CW.calibration_done) {
 * 				// read sensor data
 *				// AnalogRead(ch) is a user defined function to read the analog voltage on P and N channels
 *				sinP_in_LSB = AnalogRead(ch0);
 *				cosP_in_LSB = AnalogRead(ch1);
 *				sinN_in_LSB = AnalogRead(ch2);
 *				cosN_in_LSB = AnalogRead(ch3);
 *				ONECALIB_InitSensorData(sinP_in_LSB, cosP_in_LSB, sinN_in_LSB, cosN_in_LSB);
 *				ONECALIB_CalibrationFindParam(&calibration_store_params, true, false);
 * 			}
 *
 * 	//------------------- CALIBRATION FOR CCW DIRECTION -------------------------------
 * 			while (!calibration_store_params.rotation_data_CCW.calibration_done) {
 * 				// read sensor data
 *				// AnalogRead(ch) is a user defined function to read the analog voltage on P and N channels
 *				sinP_in_LSB = AnalogRead(ch0);
 *				cosP_in_LSB = AnalogRead(ch1);
 *				sinN_in_LSB = AnalogRead(ch2);
 *				cosN_in_LSB = AnalogRead(ch3);
 *				ONECALIB_InitSensorData(sinP_in_LSB, cosP_in_LSB, sinN_in_LSB, cosN_in_LSB);
 *				ONECALIB_CalibrationFindParam(&calibration_store_params, false, true);
 * 			}
 *
 * 	//------------------- GET CALIBRATED ANGLE DATA ----------------------------------
 * 			if (calibration_store_params.full_calibration_performed) {
 * 				angle_read = ONECALIB_GetCalibAngle(&calibration_store_params);
 * 			}
 * 			// or  angle_read = ONECALIB_GetAngle();
 * 			// Returns the calibrated or uncalibrated angle in degrees.
 *
 * 			if( SME1_angleComparison_OneTimeCompensation(&calibration_store_params))
 *               {
 *                   safetyPasses++;
 *               }
 *               else
 *               {
 *                   safetyFails++;
 *               }
 *
 *              if( SME2_1_vectorLengthCheck_OneTimeCompensation(&calibration_store_params))
 *              {
 *                  safetyPasses++;
 *              }
 *              else
 *              {
 *                  safetyFails++;
 *              }
 *
 *              if( SME2_2_vectorLengthCheck_OneTimeCompensation(&calibration_store_params))
 *              {
 *                  safetyPasses++;
 *              }
 *              else
 *              {
 *                  safetyFails++;
 *              }
 *
 *              if( SME2_3_vectorLengthCheck_OneTimeCompensation(&calibration_store_params))
 *              {
 *                  safetyPasses++;
 *              }
 *              else
 *              {
 *                  safetyFails++;
 *              }
 *
 *              if( SME3_commonModeCheck_OneTimeCompensation(&calibration_store_params))
 *              {
 *                  safetyPasses++;
 *              }
 *              else
 *              {
 *                  safetyFails++;
 *              }
 *		}
 * }
 * \endcode
 *
 * \section example_sec2 Example 2
 *
 * \dot
 *  digraph usage_graph
 *  {
 *  	fontname="Helvetica";
 *  	init [label = "ONECALIB_InitCalibData"];
 *  	readout [label = "Your periodic readout function for SIN & COS signals", shape=box, style=filled, color=".7 .3 1.0"];
 *  	init_sensor_data [label = "ONECALIB_InitSensorData"];
 *		angle_get [label = "ONECALIB_GetAngle"];
 *
 *  	init -> readout [weight=8];
 *  	readout -> init_sensor_data [color = red];
 *  	init_sensor_data -> angle_get [color = red];
 *  	angle_get -> readout [color = red];
 * }
 * \enddot
 *
 *
 * Code Example 2:
 *
 * \code{.c}
 * CALIB_DATA_t calibration_store_params;
 *
 * float32 differential_uncalibrated_angle_read = 0;
 * uint32_t sinP_in_LSB = 0;
 * uint32_t cosP_in_LSB = 0;
 * uint32_t sinN_in_LSB = 0;
 * uint32_t cosN_in_LSB = 0;
 *
 * int main(void)
 * {
 *  	ONECALIB_InitCalibData(&calibration_store_params);
 *
 *		while(1U)
 *		{
 *			// read sensor data
 *			// AnalogRead(ch) is a user defined function to read the analog voltage on P and N channels
 *			sinP_in_LSB = AnalogRead(ch0);
 *			cosP_in_LSB = AnalogRead(ch1);
 *			sinN_in_LSB = AnalogRead(ch2);
 *			cosN_in_LSB = AnalogRead(ch3);
 *			ONECALIB_InitSensorData(sinP_in_LSB, cosP_in_LSB, sinN_in_LSB, cosN_in_LSB);
 *			differential_uncalibrated_angle_read = ONECALIB_GetAngle();
 *		}
 * }
 * \endcode
 *
 *
 *
 * \section consider_sec Other considerations
 *
 * \see https://www.infineon.com/dgdl/Infineon-TLE5xxx(D)_Calibration_360_AN-v02_00-AN-v02_00-EN.pdf?fileId=5546d46264a8de7e0164f09d8bfa228d
 * \this example has been implemented by using the AURIX-lite-Kit TCxx4. Please be aware that ADC configuration will be different if another MCU is used.
 */

#include "onecalib.h"

// Sensor variant dependent defines. Current values are only usable with TLE5502
#define SME_VAL1  4.2
#define SME_VAL2_LOW  0.76
#define SME_VAL2_HIGH  1.24
#define SME_VAL3_VCM_POS  0.08
#define SME_VAL3_VCM_NEG  -0.08

static ANALOG_SENSOR_DATA_t sensor_data;

static void ONECALIB_InitRotationDataStore(ROTATION_DATA_STORE_t *sample_store);
static void ONECALIB_CalculateDiff(void);
static void ONECALIB_AngleUncalibDiffCalculate(void);
static float ONECALIB_ONECALIBDiffCalculate(const CALIB_DATA_t *calib_param);
static void ONECALIB_CalibOneDirectionMinMax(ROTATION_DATA_STORE_t* rotation_data_store);
static void ONECALIB_CalibFindMinMax(ROTATION_DATA_STORE_t *rotation_data_store);
static void ONECALIB_CalibCalculateOneDirection(ROTATION_DATA_STORE_t *rotation_data_store);
static void ONECALIB_CalibCalculateOrtho(CALIB_DATA_t *calib_param);

/**
 * @brief
 * Function used for ANALOG_SENSOR_DATA_t data assignment.
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
void ONECALIB_InitSensorData(float sinP_in_LSB, float cosP_in_LSB, float sinN_in_LSB, float cosN_in_LSB) {
    sensor_data.cosP_LSB = cosP_in_LSB;
    sensor_data.cosN_LSB = cosN_in_LSB;
    sensor_data.sinP_LSB = sinP_in_LSB;
    sensor_data.sinN_LSB = sinN_in_LSB;
    sensor_data.diff_X = 0;
    sensor_data.diff_Y = 0;
    sensor_data.uncalibratedAngle = 0.00;

}

/**
 * @brief
 * Function used for ROTATION_DATA_STORE_t data initialization.
 * @details
 * This function will be called internally by ONECALIB_InitCalibData(CALIB_DATA_t *calib_param).
 * @param[in]
 * sample_store is a pointer of type ROTATION_DATA_STORE_t.
 * @return
 * Nothing
 */
static void ONECALIB_InitRotationDataStore(ROTATION_DATA_STORE_t *sample_store) {
    sample_store->angle135found = false;
    sample_store->angle225found = false;
    sample_store->angle45found = false;
    sample_store->angle315found = false;

    sample_store->calibration_done = false;

    sample_store->maxCosN = 0;
    sample_store->minCosN = 4096;
    sample_store->maxSinN = 0;
    sample_store->minSinN = 4096;
    sample_store->maxCosP = 0;
    sample_store->minCosP = 4096;
    sample_store->maxSinP = 0;
    sample_store->minSinP = 4096;

    sample_store->X45 = 0.00;
    sample_store->X135 = 0.00;

    sample_store->Y45 = 0.00;
    sample_store->Y135 = 0.00;

    sample_store->amplitudeXN = 0.00;
    sample_store->amplitudeYN = 0.00;
    sample_store->amplitudeXP = 0.00;
    sample_store->amplitudeYP = 0.00;


    sample_store->magnitude45 = 0.00;
    sample_store->magnitude135 = 0.00;

    sample_store->ortho_one_dir = 0.00;
}

/**
 * @brief
 * Function used for CALIB_DATA_t data initialization.
 * The pointer of type CALIB_DATA_t will be used to store calibration parameters.
 * @details
 * This function must be called once before ONECALIB_CalibrationFindParam(..).
 * @param[in]
 * calib_param is a pointer of type CALIB_DATA_t.
 * @return
 * Nothing
 */
void ONECALIB_InitCalibData(CALIB_DATA_t *calib_param) {
    ONECALIB_InitRotationDataStore(&(calib_param->rotation_data_CW));
    ONECALIB_InitRotationDataStore(&(calib_param->rotation_data_CCW));
    calib_param->mean_amplitudeX = 0.00;
    calib_param->mean_amplitudeY = 0.00;
    calib_param->mean_offsetX = 0.00;
    calib_param->mean_offsetY = 0.00;
    calib_param->cos_ortho = 0.00;
    calib_param->sin_ortho = 0.00;
    calib_param->full_calibration_performed = false;
}

/**
 * @brief
 * The function will calculate the differential values of sin and cos.
 * @details
 * The differential values will be used to get the uncalibrated angle value or in the calibration procedure.
 * It's called internally by ONECALIB_GetUncalibAngle().
 * @return
 * Nothing
 */
static void ONECALIB_CalculateDiff(void) {
    sensor_data.diff_Y = (int32_t) sensor_data.sinP_LSB - (int32_t) sensor_data.sinN_LSB;
    sensor_data.diff_X = (int32_t) sensor_data.cosP_LSB - (int32_t) sensor_data.cosN_LSB;
}

/**
 * @brief
 * The function calculates the uncalibrated angle value.
 * @return
 * Nothing
 */
static void ONECALIB_AngleUncalibDiffCalculate(void) {
    // ONECALIB_DiffCalculate(sensor_data);
    //sensor_data.uncalibratedAngle = atan2((float) sensor_data.diff_Y, (float) sensor_data.diff_X) + ANGLE180_RAD;
    sensor_data.uncalibratedAngle = atan2((float) sensor_data.diff_Y, (float) sensor_data.diff_X);
}

/**
 * @brief
 * Calculate the calibrated angle value.
 * @details
 * This function must be called only if the all calibration parameters have been extracted with ONECALIB_CalibrationFindParam(..).
 * In this case calib_param.full_calibration_performed should be true.
 * It uses the stored calibration parameters to calculate the calibrated angle value.
 * The function is called internally by ONECALIB_GetAngle(..).
 * @param[in]
 * calib_param is a pointer of type CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Returns the calibrated angle value in radians.
 */
static float ONECALIB_ONECALIBDiffCalculate(const CALIB_DATA_t *calib_param) {
    float corr_X = ((float) sensor_data.diff_X - calib_param->mean_offsetX) / calib_param->mean_amplitudeX;
    float corr_Y = ((float) sensor_data.diff_Y - calib_param->mean_offsetY) / calib_param->mean_amplitudeY;

    float ortho_Y = (corr_Y - (corr_X * calib_param->sin_ortho)) / calib_param->cos_ortho;

    //return (atan2(ortho_Y, corr_X) + ANGLE180_RAD);
    return (atan2(ortho_Y, corr_X));
}

/**
 * @brief
 * Calculates the calibrated angle if calibration has been performed, else the uncalibrated angle.
 * @details
 * This function checks the stored calibration parameters and returns a angle value, no matter if the calibration procedure has been performed.
 * @param[in]
 * calib_param is a pointer of type CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Returns the calibrated or uncalibrated angle in degrees.
 */
float ONECALIB_GetAngle(const CALIB_DATA_t *calib_param) {

    float angle = 0.00;

    ONECALIB_CalculateDiff();
    if (calib_param->full_calibration_performed)
    {
        angle = (ONECALIB_ONECALIBDiffCalculate(calib_param)) * RAD2DEGFACTOR;
    } else
    {
        ONECALIB_AngleUncalibDiffCalculate();
        angle = (sensor_data.uncalibratedAngle) * RAD2DEGFACTOR;
    }

    return angle;
}

/**
 * @brief
 * Calculates the uncalibrated angle.
 * @details
 * This function uses differential sensor data to extract the angle.
 * @return
 * Returns the uncalibrated angle in degrees.
 */
float ONECALIB_GetUncalibAngle(void) {
    ONECALIB_CalculateDiff();
    ONECALIB_AngleUncalibDiffCalculate();
    return (sensor_data.uncalibratedAngle) * RAD2DEGFACTOR;
}

/**
 * @brief
 * Calculates the calibrated angle.
 * @details
 * Don't use this if the calibration procedure has not been performed.
 * This function dosen't check the stored calibration parameters and returns the calibrated angle value.
 * @param[in]
 * calib_param is a pointer of type CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Returns the calibrated angle in degrees.
 */
float ONECALIB_GetCalibAngle(const CALIB_DATA_t *calib_param) {
    ONECALIB_CalculateDiff();
    return (ONECALIB_ONECALIBDiffCalculate(calib_param)) * RAD2DEGFACTOR;
}

/**
 * @brief
 * This function is used to extract the calibration parameters.
 * It should called until calib_param.full_calibration_performed is true.
 * To extract the correct rotation, the input parameter rotate_CW should be true until
 * calib_param.rotation_data_CW.calibration_done is true(while the magnet is spinning in CW direction).
 * Same for the CCW direction.
 * While calib_param.full_calibration_performed is false only the differential uncalibrated angle is available.
 * @details
 * This function will extract the maximum, minimum voltage levels, amplitude, offset, and orthogonality.
 * Based on the found values, it will calculate the necessary calibration parameters.
 *
 *
 * Usage example:
 * \code{.c}
 *
 * ------------------- CALIBRATION FOR CW DIRECTION -------------------------------
 * while (!calibration_store_params.rotation_data_CW.calibration_done) {
 * 	// read sensor data
 * 	ONECALIB_InitSensorData(..);
 * 	ONECALIB_CalibrationFindParam(&calibration_store_params, true,
 * 	false);
 * }
 *
 * ------------------- CALIBRATION FOR CCW DIRECTION -------------------------------
 * UART_Transmit(&UART_0, message_ccw, sizeof(message_ccw) - 1);
 * while (!calibration_store_params.rotation_data_CCW.calibration_done) {
 * 	// read sensor data
 * 	ONECALIB_InitSensorData(..);
 * 	ONECALIB_CalibrationFindParam(&calibration_store_params,
 * 	false, true);
 * }
 *
 * ------------------- GET CALIBRATED ANGLE DATA ----------------------------------
 * if (calibration_store_params.full_calibration_performed) {
 * 	angle_read = ONECALIB_GetCalibAngle(&calibration_store_params);
 * }
 *
 *\endcode
 *
 * @param[in]
 * calib_param is a pointer of type CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @param[in]
 * rotate_CW is a bool that is true if the rotation is in clockwise direction.
 * @param[in]
 * rotate_CCW is a bool that is true if the rotation is in counter-clockwise direction.
 * @return
 * Nothing
 */
void ONECALIB_CalibrationFindParam(CALIB_DATA_t *calib_param, bool rotate_CW,
        bool rotate_CCW) {
    if (!calib_param->rotation_data_CW.calibration_done && rotate_CW)
    {
        // rotate CW
        ONECALIB_CalculateDiff();
        ONECALIB_AngleUncalibDiffCalculate();
        ONECALIB_CalibOneDirectionMinMax(&calib_param->rotation_data_CW);
    } else if (!calib_param->rotation_data_CCW.calibration_done && rotate_CCW)
    {
        // rotate CCW
        ONECALIB_CalculateDiff();
        ONECALIB_AngleUncalibDiffCalculate();
        ONECALIB_CalibOneDirectionMinMax(&calib_param->rotation_data_CCW);
    } else
    {

    }

    if (calib_param->rotation_data_CCW.calibration_done && calib_param->rotation_data_CW.calibration_done)
    {
        ONECALIB_CalibCalculateOrtho(calib_param);
        calib_param->full_calibration_performed = true;
    }
}

/**
 * @brief
 * Finds and Stores ROTATION_DATA_STORE_t parameters.
 * @details
 * This function is called internally by ONECALIB_CalibrationFindParam().
 * Used to search for the magnitude components (at 45 and 135 deg) and min/max signal level.
 * After the parameters have been found, it will calculate the correction parameters with ONECALIB_CalibCalculateOneDirection().
 * @param[in]
 * rotation_data_store is a pointer of type ROTATION_DATA_STORE_t, this structure holds the current rotation parameters.
 * @return
 * Nothing
 */
static void ONECALIB_CalibOneDirectionMinMax(ROTATION_DATA_STORE_t* rotation_data_store) {
    ONECALIB_CalibFindMinMax(rotation_data_store);

    //if(((float)ANGLE45_H) <= ((float)sensor_data.uncalibratedAngle))

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
    } else if ((sensor_data.uncalibratedAngle <= ANGLE225_H) && (sensor_data.uncalibratedAngle >= ANGLE225_L))
    {
        rotation_data_store->angle225found = true;
    } else if ((sensor_data.uncalibratedAngle <= ANGLE315_H) && (sensor_data.uncalibratedAngle >= ANGLE315_L))
    {
        rotation_data_store->angle315found = true;
    } else
    {

    }

    if (rotation_data_store->angle45found && rotation_data_store->angle135found && rotation_data_store->angle225found
            && rotation_data_store->angle315found)
    {
        ONECALIB_CalibCalculateOneDirection(rotation_data_store);
        rotation_data_store->calibration_done = true;
    } else
    {

    }
}

/**
 * @brief
 * Finds and Stores ROTATION_DATA_STORE_t parameters.
 * @details
 * Used to search for the min/max differential signal level.
 * This function is called internally by ONECALIB_CalibOneDirectionMinMax().
 * @param[in]
 * rotation_data_store is a pointer of type ROTATION_DATA_STORE_t, this structure holds the current rotation parameters.
 * @return
 * Nothing
 */
static void ONECALIB_CalibFindMinMax(ROTATION_DATA_STORE_t *rotation_data_store) {

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
 * Calculates and stores ROTATION_DATA_STORE_t parameters, based on found parameters.
 * @details
 * Should be called only if rotation_data_store->calibration_done is true.
 * This function is called internally by ONECALIB_CalibOneDirectionMinMax().
 * Calculates the amplitude, offset, magnitude and orthogonality for one direction (CW or CCW).
 * @param[in]
 * rotation_data_store is a pointer of type ROTATION_DATA_STORE_t, this structure holds the current rotation parameters.
 * @return
 * Nothing
 */
static void ONECALIB_CalibCalculateOneDirection(ROTATION_DATA_STORE_t *rotation_data_store) {
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
 * Calculates and stores CALIB_DATA_t parameters, based on found parameters from both CW and CCW rotations.
 * @details
 * Should be called only if calib_param->full_calibration_performed is true.
 * This function is called internally by ONECALIB_CalibrationFindParam(..).
 * Calculates the mean amplitude, offset and orthogonality sin and cos constants, based on found parameters from both CW and CCW rotations.
 * @param[in]
 * calib_param is a pointer of type CALIB_DATA_t, this structure holds the current sensor calibration parameters.
 * @return
 * Nothing
 */
static void ONECALIB_CalibCalculateOrtho(CALIB_DATA_t *calib_param) {
    calib_param->mean_amplitudeX = (calib_param->rotation_data_CW.amplitudeX + calib_param->rotation_data_CCW.amplitudeX) / 2.00;
    calib_param->mean_amplitudeY = (calib_param->rotation_data_CW.amplitudeY + calib_param->rotation_data_CCW.amplitudeY) / 2.00;

    calib_param->mean_offsetX = (calib_param->rotation_data_CW.offsetX + calib_param->rotation_data_CCW.offsetX) / 2.00;
    calib_param->mean_offsetY = (calib_param->rotation_data_CW.offsetY + calib_param->rotation_data_CCW.offsetY) / 2.00;

    calib_param->sin_ortho = sin(-((calib_param->rotation_data_CW.ortho_one_dir + calib_param->rotation_data_CCW.ortho_one_dir) / 2.00));
    calib_param->cos_ortho = cos(-((calib_param->rotation_data_CW.ortho_one_dir + calib_param->rotation_data_CCW.ortho_one_dir) / 2.00));
}


/*
Check whether the difference of the calculated angle signals αN and αP is inside
the range of [-αthreshold, αthreshold]

This function uses the stored calibration parameters to calculate the calibrated angle values. These values are then used to check if their difference is in the range of [-threshold, threshold]
This performs the Safety Mechanism External 1

Returns: false for fail; true for pass
 */
bool SME1_angleComparison_OneTimeCompensation(const CALIB_DATA_t *calib_param )
{

    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data_CW.offsetXP) / calib_param->rotation_data_CW.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data_CW.offsetYP) / calib_param->rotation_data_CW.amplitudeYP;
    volatile float CosN_display_calibrated = ( (float)sensor_data.cosN_LSB - calib_param->rotation_data_CW.offsetXN) / calib_param->rotation_data_CW.amplitudeXN;
    volatile float SinN_display_calibrated = ( (float)sensor_data.sinN_LSB - calib_param->rotation_data_CW.offsetYN) / calib_param->rotation_data_CW.amplitudeYN;

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


bool SME2_1_vectorLengthCheck_OneTimeCompensation(const CALIB_DATA_t *calib_param)
{
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data_CW.offsetXP) / calib_param->rotation_data_CW.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data_CW.offsetYP) / calib_param->rotation_data_CW.amplitudeYP;
    volatile float CosN_display_calibrated = ( (float)sensor_data.cosN_LSB - calib_param->rotation_data_CW.offsetXN) / calib_param->rotation_data_CW.amplitudeXN;
    volatile float SinN_display_calibrated = ( (float)sensor_data.sinN_LSB - calib_param->rotation_data_CW.offsetYN) / calib_param->rotation_data_CW.amplitudeYN;


    volatile float SinP_ortho_corrected = (float)( (SinP_display_calibrated - (CosP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float CosP_ortho_corrected = (float)( (CosP_display_calibrated - (SinP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float SinN_ortho_corrected = (float)( (SinN_display_calibrated - (CosN_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float CosN_ortho_corrected = (float)( (CosN_display_calibrated - (SinN_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);


    volatile float V_P = sqrt( pow(CosP_ortho_corrected,2) + pow(SinP_ortho_corrected,2)  );
    volatile float V_N = sqrt( pow(CosN_ortho_corrected,2) + pow(SinN_ortho_corrected,2)  );

    if(V_P >= SME_VAL2_LOW && V_P < SME_VAL2_HIGH && V_N >= SME_VAL2_LOW && V_N < SME_VAL2_HIGH )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
 * Check whether the calculated signal V_P = SQRT((SIN_P)2 + (COS_P)2) and
V_N = SQRT((SIN_N)^2 + (COS_N)^2) are both inside the range of [rmin, rmax]

returns false true for pass; false for fail
 */



bool SME2_2_vectorLengthCheck_OneTimeCompensation(const CALIB_DATA_t *calib_param)
{
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data_CW.offsetXP) / calib_param->rotation_data_CW.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data_CW.offsetYP) / calib_param->rotation_data_CW.amplitudeYP;


    volatile float SinP_ortho_corrected = (float)( (SinP_display_calibrated - (CosP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);
    volatile float CosP_ortho_corrected = (float)( (CosP_display_calibrated - (SinP_display_calibrated * calib_param->sin_ortho)) / calib_param->cos_ortho);


    volatile float V_P = sqrt( pow(CosP_ortho_corrected,2) + pow(SinP_ortho_corrected,2)  );

    if(V_P >= SME_VAL2_LOW && V_P < SME_VAL2_HIGH)
    {
        return true;
    }
    else
    {
        return false;
    }

}

/*
 * Check whether the calculated signal
V = SQRT((SIN_P-SIN_N)^2 + (COS_P-COS_N)^2)
is inside the range of [rmin, rmaxr]].

returns false true for pass; false for fail
 */
bool SME2_3_vectorLengthCheck_OneTimeCompensation(const CALIB_DATA_t *calib_param)
{
    //scaled to 5 volts reference and 12 bits resolution
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data_CW.offsetXP) / calib_param->rotation_data_CW.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data_CW.offsetYP) / calib_param->rotation_data_CW.amplitudeYP;


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



/*
 * Check whether the
calculated common mode
part Vcm_s and Vcm_c of the
SIN and COS signals are
inside the range of: [Vcm_min,
Vcm_max].
Vcm_s = SIN_P + SIN_N
Vcm_c = COS_P + COS_N

Returns: true for pass, false for fail
 */

bool SME3_commonModeCheck_OneTimeCompensation(const CALIB_DATA_t *calib_param)
{
    volatile float CosP_display_calibrated = ( (float)sensor_data.cosP_LSB - calib_param->rotation_data_CW.offsetXP) / calib_param->rotation_data_CW.amplitudeXP;
    volatile float SinP_display_calibrated = ( (float)sensor_data.sinP_LSB - calib_param->rotation_data_CW.offsetYP) / calib_param->rotation_data_CW.amplitudeYP;
    volatile float CosN_display_calibrated = ( (float)sensor_data.cosN_LSB - calib_param->rotation_data_CW.offsetXN) / calib_param->rotation_data_CW.amplitudeXN;
    volatile float SinN_display_calibrated = ( (float)sensor_data.sinN_LSB - calib_param->rotation_data_CW.offsetYN) / calib_param->rotation_data_CW.amplitudeYN;

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

