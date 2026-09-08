/**********************************************************************************************************************
 * \file adc_sync4_Ccu.h
 * \brief Synchronous 4-Channel ADC Driver with CCU60 Triggering - Header File
 * \details This module provides an interface for parallel ADC conversions across 4 VADC groups,
 *          triggered synchronously by CCU60 Timer T12. Designed specifically for TLE5502 magnetic
 *          position sensor readout requiring simultaneous sampling of all differential outputs.
 *
 * \author Infineon Technologies AG
 * \copyright Copyright (C) Infineon Technologies AG 2025
 *
 * \note Hardware Dependencies:
 *       - AURIX TC297 or compatible derivative with 4 VADC groups
 *       - CCU60 module for trigger generation
 *       - TLE5502 sensor connected to AN3, AN8, AN20, AN24
 *********************************************************************************************************************/

#ifndef ADC_SYNC4_Ccu_H
#define ADC_SYNC4_Ccu_H

/* ==================== Includes ==================== */
#include "IfxVadc_Adc.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Type Definitions ==================== */

/**
 * \brief Synchronous 4-channel ADC data structure
 * \details Contains all necessary handles and data for managing parallel ADC conversions
 *          across 4 VADC groups. Each group converts one TLE5502 sensor output channel.
 *
 * \note Channel Mapping:
 *       - an3_g0ch3  : AN3  (VADCG0.3) - SINP (Sine Positive)
 *       - an8_g1ch0  : AN8  (VADCG1.0) - COSP (Cosine Positive)
 *       - an20_g2ch4 : AN20 (VADCG2.4) - SINN (Sine Negative)
 *       - an24_g3ch0 : AN24 (VADCG3.0) - COSN (Cosine Negative)
 */
typedef struct
{
    /* VADC Module and Group Handles */
    IfxVadc_Adc         vadc;       /**< VADC module handle */

    /* One group per channel - enables parallel conversion */
    IfxVadc_Adc_Group   g0;         /**< VADC Group 0 handle */
    IfxVadc_Adc_Group   g1;         /**< VADC Group 1 handle */
    IfxVadc_Adc_Group   g2;         /**< VADC Group 2 handle */
    IfxVadc_Adc_Group   g3;         /**< VADC Group 3 handle */

    /* Channel Handles - mapped from hardware pins */
    IfxVadc_Adc_Channel an3_g0ch3;  /**< AN3  = VADCG0CH3 (SINP) */
    IfxVadc_Adc_Channel an8_g1ch0;  /**< AN8  = VADCG1CH0 (COSP) */
    IfxVadc_Adc_Channel an20_g2ch4; /**< AN20 = VADCG2CH4 (SINN) */
    IfxVadc_Adc_Channel an24_g3ch0; /**< AN24 = VADCG3CH0 (COSN - ISR source) */


    /* Raw ADC Result Storage (12-bit values) */
    volatile uint16     raw_an3;    /**< Latest AN3 conversion result (SINP) */
    volatile uint16     raw_an8;    /**< Latest AN8 conversion result (COSP) */
    volatile uint16     raw_an20;   /**< Latest AN20 conversion result (SINN) */
    volatile uint16     raw_an24;   /**< Latest AN24 conversion result (COSN) */

    /* Data Ready Flag */
    volatile uint8      newSet;     /**< Flag: 1 = new data available, 0 = no new data */
} AdcSync4Ccu;

/* ==================== Function Prototypes ==================== */

/**
 * \brief Initialize synchronous 4-channel ADC system with CCU60 triggering
 * \details Performs complete initialization of:
 *          - VADC module and 4 groups (G0, G1, G2, G3)
 *          - 4 ADC channels with result registers
 *          - CCU60 T12 timer configured to generate SR3 trigger
 *          - Interrupt service routine for conversion complete event
 *          - Queue configuration with external trigger and refill mode
 *
 * \param[in,out] h Pointer to AdcSync4Ccu structure (must be allocated by caller)
 *
 * \note This function must be called once during system initialization, before starting
 *       the timer with AdcSync4Ccu_start().
 *
 * \warning Ensure the AdcSync4Ccu structure is allocated before calling this function.
 *
 * \see AdcSync4Ccu_start()
 */
void AdcSync4Ccu_init(AdcSync4Ccu *h);

/**
 * \brief Start CCU60 timer to begin periodic ADC triggering
 * \details Starts the CCU60 T12 timer which generates SR3 signals that trigger all
 *          4 VADC groups simultaneously. Each SR3 pulse initiates one conversion cycle
 *          across all channels.
 *
 * \param[in] h Pointer to AdcSync4Ccu structure (can be NULL, not used internally)
 *
 * \note Call this function after AdcSync4Ccu_init() to begin continuous ADC conversions.
 *       The sampling rate is determined by the T12 period configured during initialization.
 *
 * \see AdcSync4Ccu_stop()
 */
void AdcSync4Ccu_start();

/**
 * \brief Stop CCU60 timer to halt ADC triggering
 * \details Stops the CCU60 T12 timer, halting SR3 generation and ADC conversions.
 *          Raw ADC values remain accessible but will not update until timer is restarted.
 *
 * \param[in] h Pointer to AdcSync4Ccu structure (can be NULL, not used internally)
 *
 * \note Useful during calibration phases when manual control or slower sampling is needed.
 *       Call AdcSync4Ccu_start() to resume automatic conversions.
 *
 * \see AdcSync4Ccu_start()
 */
void AdcSync4Ccu_stop();

/**
 * \brief Check and clear the new data available flag
 * \details Atomically checks if a new set of ADC conversions has completed since the
 *          last call, then clears the flag. Use this to poll for new data in the main loop.
 *
 * \param[in,out] h Pointer to AdcSync4Ccu structure
 *
 * \return uint8 Status flag
 * \retval 1 New ADC data is available (flag was set)
 * \retval 0 No new data since last check (flag was already clear)
 *
 * \note The flag is set by the ISR when AN24 conversion completes (signaling all channels done).
 *       This function provides a non-blocking way to check for data updates.
 *
 * \code
 * while(1) {
 *     if (AdcSync4Ccu_fetchNewSetFlag(&g_adc)) {
 *         // Process new ADC data
 *         sinP = AdcSync4Ccu_getRawAN3(&g_adc);
 *         // ...
 *     }
 * }
 * \endcode
 */
uint8 AdcSync4Ccu_fetchNewSetFlag(AdcSync4Ccu *h);

/**
 * \brief Manually invoke ISR handler for testing/debugging
 * \details Calls the ISR handler logic without waiting for hardware interrupt.
 *          Useful for:
 *          - Forcing data readout during calibration
 *          - Testing ISR logic without timer
 *          - Debugging conversion flow
 *
 * \param[in] h Pointer to AdcSync4Ccu structure
 *
 * \warning This does NOT trigger actual ADC conversions. It only reads the current
 *          result registers. Ensure conversions have completed before calling.
 *
 * \note Use during calibration when timer is stopped and manual control is needed.
 */
void AdcSync4Ccu_forceIsr(AdcSync4Ccu *h);

/**
 * \brief ISR handler logic (called by interrupt or forceIsr)
 * \details Reads all 4 ADC channel results, validates them using VF (Valid Flag),
 *          updates the raw value storage, and sets the newSet flag.
 *
 * \param[in,out] h Pointer to AdcSync4Ccu structure
 *
 * \note This function is called automatically by hardware interrupt when AN24
 *       conversion completes. Can also be called manually via AdcSync4Ccu_forceIsr().
 *
 * \warning Do not call this function directly unless you understand the timing implications.
 *          Use AdcSync4Ccu_forceIsr() for manual triggering.
 */
void AdcSync4Ccu_isrHandler(AdcSync4Ccu *h);

/* ==================== Inline Accessor Functions ==================== */

/**
 * \brief Get latest raw ADC value for AN3 (SINP)
 * \param[in] h Pointer to AdcSync4Ccu structure (const, read-only access)
 * \return uint16 12-bit ADC result (0-4095 LSB)
 * \note Data is updated by ISR when conversion completes
 */
static inline uint16 AdcSync4Ccu_getRawAN3(const AdcSync4Ccu *h)
{
    return h->raw_an3;
}

/**
 * \brief Get latest raw ADC value for AN8 (COSP)
 * \param[in] h Pointer to AdcSync4Ccu structure (const, read-only access)
 * \return uint16 12-bit ADC result (0-4095 LSB)
 * \note Data is updated by ISR when conversion completes
 */
static inline uint16 AdcSync4Ccu_getRawAN8(const AdcSync4Ccu *h)
{
    return h->raw_an8;
}

/**
 * \brief Get latest raw ADC value for AN20 (SINN)
 * \param[in] h Pointer to AdcSync4Ccu structure (const, read-only access)
 * \return uint16 12-bit ADC result (0-4095 LSB)
 * \note Data is updated by ISR when conversion completes
 */
static inline uint16 AdcSync4Ccu_getRawAN20(const AdcSync4Ccu *h)
{
    return h->raw_an20;
}

/**
 * \brief Get latest raw ADC value for AN24 (COSN)
 * \param[in] h Pointer to AdcSync4Ccu structure (const, read-only access)
 * \return uint16 12-bit ADC result (0-4095 LSB)
 * \note Data is updated by ISR when conversion completes
 * \note This channel triggers the ISR, so it completes last
 */
static inline uint16 AdcSync4Ccu_getRawAN24(const AdcSync4Ccu *h)
{
    return h->raw_an24;
}

#ifdef __cplusplus
}
#endif

#endif /* ADC_SYNC4_Ccu_H */
