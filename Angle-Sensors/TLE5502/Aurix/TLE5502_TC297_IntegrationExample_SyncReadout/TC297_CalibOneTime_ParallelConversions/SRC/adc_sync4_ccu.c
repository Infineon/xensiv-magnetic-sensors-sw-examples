/**********************************************************************************************************************
 * \file adc_sync4_Ccu.c
 * \brief Synchronous 4-Channel ADC Driver with CCU60 Triggering
 * \details This module implements parallel ADC conversions across 4 VADC groups, triggered synchronously
 *          by CCU60 Timer T12. Each VADC group handles one TLE5502 sensor output channel, ensuring
 *          true simultaneous sampling for accurate angle calculation.
 *
 * \author Infineon Technologies AG
 * \copyright Copyright (C) Infineon Technologies AG 2025
 *
 * \note This driver is specifically designed for TC297 with 4 independent VADC groups.
 *********************************************************************************************************************/

/* ==================== Includes ==================== */
#include <adc_sync4_ccu.h>
#include "IfxCpu.h"
#include "IfxVadc.h"
#include "IfxVadc_Adc.h"
#include "IfxScuWdt.h"
#include "IfxCcu6_reg.h"

/* ==================== Configuration Defines ==================== */

/* VADC Queue Configuration Flags */
#define IFXVADC_QUEUE_SOURCEIRQEN (1u << 6)    /**< Enable request source event interrupt */
#define IFXVADC_QUEUE_EXTR        (1u << 7)    /**< Enable external trigger mode */

/* Interrupt Configuration */
#define ADC_SYNC4_ISR_PRIO   (0)               /**< Interrupt priority (0 = highest on TC297) */
#define ADC_SYNC4_ISR_CPU    (0)               /**< CPU assignment (CPU0) */

/* EVADC Trigger Source Selection */
/**
 * \brief XTSEL encoding for CCU60_SR3 trigger
 * \details On TC297, CCU60 SR3 (Service Request 3) is encoded as 0x0 for REQTRA.
 *          Verify this value in the AURIX TC2xx User Manual, EVADC chapter,
 *          "Request Source Selection" table for your specific derivative.
 */
#define EVADC_XTSEL_CCU60_SR3   (0u)

/* CCU60 Timer Configuration */
#define CCU60_T12_PERIOD_DEFAULT  (10000u)    /**< Default T12 period (adjustable for sampling rate) */

/* ==================== Static Variables ==================== */
static AdcSync4Ccu *g_h = 0;                   /**< Global handle for ISR access */

/* ==================== Interrupt Service Routine ==================== */
/**
 * \brief ADC conversion complete interrupt handler
 * \details Triggered when AN24 (VADCG3.0) conversion completes, signaling that all 4
 *          parallel conversions are done. Reads all channel results and sets newSet flag.
 * \param[in] 0 Service provider (CPU0)
 * \param[in] ADC_SYNC4_ISR_PRIO Interrupt priority level
 */
IFX_INTERRUPT(AdcSync4Ccu_ISR, 0, ADC_SYNC4_ISR_PRIO)
{
    if (g_h != 0)
    {
        AdcSync4Ccu_isrHandler(g_h);
    }
}

/* ==================== Static Helper Functions ==================== */

/**
 * \brief Add ADC channel to Queue0 with external trigger and refill mode
 * \details Configures the channel for continuous operation with external trigger.
 *          The IFXVADC_QUEUE_REFILL flag ensures the queue automatically reloads
 *          after each conversion for continuous sampling.
 * \param[in] ch Pointer to initialized ADC channel structure
 */
static void addChannelToQueue0(IfxVadc_Adc_Channel *ch)
{
    IfxVadc_Adc_addToQueue(ch, IFXVADC_QUEUE_REFILL | IFXVADC_QUEUE_EXTR);
}

/**
 * \brief Initialize VADC group with Queue0 configured for external triggering
 * \details Sets up a VADC group with:
 *          - Queue-based conversion (no scan or background)
 *          - External trigger mode (will be patched to CCU60_SR3)
 *          - High priority request slot
 *          - Rising edge trigger mode
 *          - Always-open gating
 * \param[in] vadc Pointer to initialized VADC module
 * \param[out] group Pointer to group structure to initialize
 * \param[in] groupId VADC group identifier (0-3)
 */
static void initGroupQueue0Triggered(IfxVadc_Adc *vadc,
                                     IfxVadc_Adc_Group *group,
                                     IfxVadc_GroupId groupId)
{
    IfxVadc_Adc_GroupConfig gCfg;
    IfxVadc_Adc_initGroupConfig(&gCfg, vadc);

    /* Disable unused conversion modes */
    gCfg.backgroundScanRequest.autoBackgroundScanEnabled = FALSE;
    gCfg.scanRequest.autoscanEnabled = FALSE;
    gCfg.arbiter.requestSlotBackgroundScanEnabled = FALSE;
    gCfg.arbiter.requestSlotScanEnabled = FALSE;

    /* Configure queue-based conversion */
    gCfg.groupId = groupId;
    gCfg.arbiter.requestSlotQueueEnabled = TRUE;

    gCfg.queueRequest.flushQueueAfterInit  = TRUE;
    gCfg.queueRequest.requestSlotPrio      = IfxVadc_RequestSlotPriority_high;
    gCfg.queueRequest.requestSlotStartMode = IfxVadc_RequestSlotStartMode_waitForStart;

    /* Configure trigger mode (source will be patched after init) */
    gCfg.queueRequest.triggerConfig.gatingMode    = IfxVadc_GatingMode_always;
    gCfg.queueRequest.triggerConfig.triggerMode   = IfxVadc_TriggerMode_uponRisingEdge;
    gCfg.queueRequest.triggerConfig.triggerSource = IfxVadc_TriggerSource_0; /* Placeholder */

    IfxVadc_Adc_initGroup(group, &gCfg);
}

/**
 * \brief Patch VADC queue trigger to use CCU60_SR3 internal source
 * \details The iLLD library's IfxVadc_TriggerSource enum doesn't include CCU60_SR3,
 *          so we must directly configure the hardware registers after group initialization.
 *          This function:
 *          - Enables external trigger for Queue0
 *          - Sets rising edge trigger mode
 *          - Selects CCU60_SR3 as trigger source via REQTRA
 *          - Opens gating for continuous operation
 * \param[in,out] group Pointer to initialized VADC group
 */
static void vadcPatchQueueTriggerToCcu60Sr3(IfxVadc_Adc_Group *group)
{
    /* Enable external trigger for queue0 */
    group->group->QMR0.B.ENTR = 1;

    /* Set trigger mode (rising edge) */
    group->group->QCTRL0.B.XTMODE = IfxVadc_TriggerMode_uponRisingEdge;

    /* Select trigger source encoding (REQTRA = CCU60_SR3) */
    group->group->QCTRL0.B.XTSEL = EVADC_XTSEL_CCU60_SR3;

    /* Gate always open */
    group->group->QMR0.B.ENGT = IfxVadc_GatingMode_always;
}

/**
 * \brief Initialize CCU60 T12 timer to generate SR3 on period match
 * \details Configures CCU60 Timer T12 as a periodic timer that generates Service Request 3 (SR3)
 *          every time the counter matches the period register. This SR3 signal triggers all
 *          VADC group conversions simultaneously.
 *
 * \note T12 period can be adjusted based on required sampling frequency:
 *       f_sample = f_CCU6 / t12Period
 *       Example: If f_CCU6 = 100 MHz and t12Period = 10000, then f_sample = 10 kHz
 *
 * \param[in] t12Period Timer period in clock cycles (determines sampling rate)
 */
static void initCcu60_T12_PeriodMatch_Sr3(uint16 t12Period)
{
    uint16 passwd = IfxScuWdt_getCpuWatchdogPassword();

    /* Enable CCU60 module clock */
    IfxScuWdt_clearCpuEndinit(passwd);
    MODULE_CCU60.CLC.B.DISR = 0;  /* Disable request = 0 (enable module) */
    IfxScuWdt_setCpuEndinit(passwd);

    /* Wait for module to be enabled */
    while (MODULE_CCU60.CLC.B.DISS != 0) { }

    /* Disable all interrupt enables before configuration */
    MODULE_CCU60.IEN.U = 0;

    /* Configure T12 period and reset counter */
    MODULE_CCU60.T12PR.U = t12Period;  /* Set period register */
    MODULE_CCU60.T12.U   = 0;          /* Reset counter to 0 */

    /* Route T12 period match event to SR3 output
     * INP.INPT12 encoding:
     *   0 = SR0, 1 = SR1, 2 = SR2, 3 = SR3
     */
    MODULE_CCU60.INP.B.INPT12 = 3;

    /* Enable T12 period match interrupt (generates SR3 signal) */
    MODULE_CCU60.IEN.B.ENT12PM = 1;

    /* Start T12 timer
     * T12RS  = Run Set (enable counter)
     * T12STR = Shadow Transfer Request (load period register)
     */
    MODULE_CCU60.TCTR4.B.T12RS  = 1;
    MODULE_CCU60.TCTR4.B.T12STR = 1;
}

/* ==================== Public API Functions ==================== */

/**
 * \brief Initialize synchronous 4-channel ADC system with CCU60 triggering
 * \details Performs complete initialization of:
 *          - VADC module and 4 groups (G0, G1, G2, G3)
 *          - 4 ADC channels (AN3, AN8, AN20, AN24)
 *          - CCU60 T12 timer configured to generate SR3 trigger
 *          - Interrupt service routine for conversion complete event
 *
 * \note Call this function once during system initialization, before AdcSync4Ccu_start()
 *
 * \param[in,out] h Pointer to AdcSync4Ccu structure (must be allocated by caller)
 */
void AdcSync4Ccu_init(AdcSync4Ccu *h)
{
    g_h = h;  /* Store global handle for ISR access */

    /* Initialize data members */
    h->newSet   = 0;
    h->raw_an3  = 0;
    h->raw_an8  = 0;
    h->raw_an20 = 0;
    h->raw_an24 = 0;

    /* ==================== VADC Module Initialization ==================== */
    {
        IfxVadc_Adc_Config vadcCfg;
        IfxVadc_Adc_initModuleConfig(&vadcCfg, &MODULE_VADC);
        IfxVadc_Adc_initModule(&h->vadc, &vadcCfg);
    }

    /* ==================== VADC Groups Initialization ==================== */
    /* Configure all 4 groups with queue-based external trigger mode */
    initGroupQueue0Triggered(&h->vadc, &h->g0, IfxVadc_GroupId_0);
    initGroupQueue0Triggered(&h->vadc, &h->g1, IfxVadc_GroupId_1);
    initGroupQueue0Triggered(&h->vadc, &h->g2, IfxVadc_GroupId_2);
    initGroupQueue0Triggered(&h->vadc, &h->g3, IfxVadc_GroupId_3);

    /* Patch all groups to use CCU60 SR3 as trigger source */
    vadcPatchQueueTriggerToCcu60Sr3(&h->g0);
    vadcPatchQueueTriggerToCcu60Sr3(&h->g1);
    vadcPatchQueueTriggerToCcu60Sr3(&h->g2);
    vadcPatchQueueTriggerToCcu60Sr3(&h->g3);

    /* ==================== ADC Channels Initialization ==================== */
    {
        IfxVadc_Adc_ChannelConfig cCfg;

        /* AN3 = G0CH3 (SINP) */
        IfxVadc_Adc_initChannelConfig(&cCfg, &h->g0);
        cCfg.channelId      = IfxVadc_ChannelId_3;
        cCfg.resultRegister = (IfxVadc_ChannelResult)3;
        IfxVadc_Adc_initChannel(&h->an3_g0ch3, &cCfg);

        /* AN8 = G1CH0 (COSP) */
        IfxVadc_Adc_initChannelConfig(&cCfg, &h->g1);
        cCfg.channelId      = IfxVadc_ChannelId_0;
        cCfg.resultRegister = (IfxVadc_ChannelResult)0;
        IfxVadc_Adc_initChannel(&h->an8_g1ch0, &cCfg);

        /* AN20 = G2CH4 (SINN) */
        IfxVadc_Adc_initChannelConfig(&cCfg, &h->g2);
        cCfg.channelId      = IfxVadc_ChannelId_4;
        cCfg.resultRegister = (IfxVadc_ChannelResult)4;
        IfxVadc_Adc_initChannel(&h->an20_g2ch4, &cCfg);

        /* AN24 = G3CH0 (COSN) - Interrupt source */
        IfxVadc_Adc_initChannelConfig(&cCfg, &h->g3);
        cCfg.channelId      = IfxVadc_ChannelId_0;
        cCfg.resultRegister = (IfxVadc_ChannelResult)0;

        /* Configure interrupt for conversion complete notification */
        cCfg.resultPriority     = ADC_SYNC4_ISR_PRIO;
        cCfg.resultServProvider = (IfxSrc_Tos)ADC_SYNC4_ISR_CPU;
        IfxVadc_Adc_initChannel(&h->an24_g3ch0, &cCfg);
    }

    /* ==================== Add Channels to Queues ==================== */
    /* Each channel is added to its respective group's Queue0 */
    addChannelToQueue0(&h->an3_g0ch3);
    addChannelToQueue0(&h->an8_g1ch0);
    addChannelToQueue0(&h->an20_g2ch4);
    addChannelToQueue0(&h->an24_g3ch0);

    /* ==================== CCU60 Timer Initialization ==================== */
    /**
     * Initialize CCU60 T12 to generate SR3 trigger at specified period.
     * Adjust period for desired sampling rate:
     *   Example: 10000 cycles @ 100 MHz CCU6 clock = 10 kHz sampling
     */
    initCcu60_T12_PeriodMatch_Sr3(CCU60_T12_PERIOD_DEFAULT);
}

/**
 * \brief Start CCU60 timer to begin periodic ADC triggering
 * \details Starts the CCU60 T12 timer, which generates SR3 signals that trigger
 *          all 4 VADC groups simultaneously. Call this after AdcSync4Ccu_init()
 *          to begin continuous ADC conversions.
 * \param[in] h Pointer to AdcSync4Ccu structure (can be NULL, not used)
 */
void AdcSync4Ccu_start()
{


    /* Start T12 timer */
    MODULE_CCU60.TCTR4.B.T12RS  = 1;  /* Run Set: Enable counter */
    MODULE_CCU60.TCTR4.B.T12STR = 1;  /* Shadow Transfer: Load configuration */
}

/**
 * \brief Stop CCU60 timer to halt ADC triggering
 * \details Stops the CCU60 T12 timer, halting SR3 generation and ADC conversions.
 *          Useful during calibration phases when manual control is needed.
 * \param[in] h Pointer to AdcSync4Ccu structure (can be NULL, not used)
 */
void AdcSync4Ccu_stop(AdcSync4Ccu *h)
{
    (void)h;  /* Unused parameter */

    /* Stop T12 timer */
    MODULE_CCU60.TCTR4.B.T12RR = 1;  /* Run Reset: Disable counter */
}

/**
 * \brief Check and clear the new data available flag
 * \details Checks if a new set of ADC conversions has completed since the last call.
 *          The flag is automatically cleared after reading.
 * \param[in,out] h Pointer to AdcSync4Ccu structure
 * \return 1 if new data is available, 0 otherwise
 */
uint8 AdcSync4Ccu_fetchNewSetFlag(AdcSync4Ccu *h)
{
    uint8 f = h->newSet;
    h->newSet = 0;  /* Clear flag after reading */
    return f;
}

/**
 * \brief Manually trigger ISR handler for testing/debugging
 * \details Calls the ISR handler logic without waiting for hardware interrupt.
 *          Useful during calibration for manual data acquisition.
 * \warning This does NOT trigger actual ADC conversions, it only reads current results.
 * \param[in] h Pointer to AdcSync4Ccu structure
 */
void AdcSync4Ccu_forceIsr(AdcSync4Ccu *h)
{
    AdcSync4Ccu_isrHandler(h);
}

/**
 * \brief ISR handler logic - reads all ADC results and sets new data flag
 * \details Called by hardware interrupt when AN24 conversion completes, or manually
 *          via AdcSync4Ccu_forceIsr(). Reads all 4 channel results and validates
 *          data using the VF (Valid Flag) bit.
 *
 * \note The VF bit indicates that a new conversion result is available and has not
 *       been read yet. If VF=0, the previous value is retained.
 *
 * \param[in,out] h Pointer to AdcSync4Ccu structure
 */
void AdcSync4Ccu_isrHandler(AdcSync4Ccu *h)
{
    Ifx_VADC_RES r;  /* Result register structure */

    /* Read AN3 (SINP) */
    r = IfxVadc_Adc_getResult(&h->an3_g0ch3);
    if (r.B.VF)  /* Valid Flag: new data available */
    {
        h->raw_an3 = (uint16)r.B.RESULT;
    }

    /* Read AN8 (COSP) */
    r = IfxVadc_Adc_getResult(&h->an8_g1ch0);
    if (r.B.VF)
    {
        h->raw_an8 = (uint16)r.B.RESULT;
    }

    /* Read AN20 (SINN) */
    r = IfxVadc_Adc_getResult(&h->an20_g2ch4);
    if (r.B.VF)
    {
        h->raw_an20 = (uint16)r.B.RESULT;
    }

    /* Read AN24 (COSN) */
    r = IfxVadc_Adc_getResult(&h->an24_g3ch0);
    if (r.B.VF)
    {
        h->raw_an24 = (uint16)r.B.RESULT;
    }

    /* Set flag indicating new data set is available */
    h->newSet = 1;
}
