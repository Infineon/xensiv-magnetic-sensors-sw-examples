#include "Ifx_Types.h"
#include "IfxPort.h"
#include "IfxEvadc_Adc.h"
#include "Evadc/Std/IfxEvadc.h"  // for IFXEVADC_QUEUE_REFILL, gating helpers, etc.

/*
  Mapping (all on Group 8):
    idx 0 -> Cos N : G8CH5 (P40.7)
    idx 1 -> Sin N : G8CH4 (P40.6)
    idx 2 -> Cos P : G8CH1 (P40.5)
    idx 3 -> Sin P : G8CH0 (P40.4)
*/

typedef struct
{
    IfxEvadc_Adc         module;
    IfxEvadc_Adc_Group   g8;
    IfxEvadc_Adc_Channel ch[4];
} EvadcG8App;

static EvadcG8App s_eva;

/* Logical order you requested; all belong to Group 8 */
static const IfxEvadc_GroupId   kGroupId = IfxEvadc_GroupId_8;
static const IfxEvadc_ChannelId kChIds[4] =
{
    IfxEvadc_ChannelId_5,  // idx 0: G8CH5 -> P40.7
    IfxEvadc_ChannelId_4,  // idx 1: G8CH4 -> P40.6
    IfxEvadc_ChannelId_1,  // idx 2: G8CH1 -> P40.5
    IfxEvadc_ChannelId_0   // idx 3: G8CH0 -> P40.4
};

static const IfxEvadc_RequestSource kReqSrc = IfxEvadc_RequestSource_queue0;

/* Configure P40.<pinIdx> as analog-friendly input (no pull device) */
static inline void adc_configure_p40_pin(uint8 pinIdx)
{
    IfxPort_setPinModeInput(&MODULE_P40, pinIdx, IfxPort_InputMode_noPullDevice);
}

/* Initialize EVADC module, Group 8, and 4 channels; load queue0 with refill and start it */
void evadc_g8_init_queue_continuous(void)
{
    /* 1) Put pads into analog-friendly mode */
    adc_configure_p40_pin(7); /* P40.7 -> G8CH5 */
    adc_configure_p40_pin(6); /* P40.6 -> G8CH4 */
    adc_configure_p40_pin(5); /* P40.5 -> G8CH1 */
    adc_configure_p40_pin(4); /* P40.4 -> G8CH0 */

    /* 2) EVADC module init */
    IfxEvadc_Adc_Config mCfg;
    IfxEvadc_Adc_initModuleConfig(&mCfg, &MODULE_EVADC);
    /* Optional global settings:
       mCfg.analogClockGenerationMode = IfxEvadc_AnalogClockGenerationMode_normal; etc. */
    IfxEvadc_Adc_initModule(&s_eva.module, &mCfg);

    /* 3) Group 8 init: enable queue0, gate “always” (free-run), master = groupId */
    IfxEvadc_Adc_GroupConfig gCfg;
    IfxEvadc_Adc_initGroupConfig(&gCfg, &s_eva.module);
    gCfg.groupId = kGroupId;
    gCfg.master  = gCfg.groupId;

    /* Enable queue request slots you plan to use; here we use queue0 */
    gCfg.arbiter.requestSlotQueue0Enabled = TRUE;

    /* Gate/trigger for queue0: set gate to “always” for autonomous operation */
    gCfg.queueRequest[0].triggerConfig.gatingMode   = IfxEvadc_GatingMode_always;
    gCfg.queueRequest[0].triggerConfig.triggerMode  = IfxEvadc_TriggerMode_noExternalTrigger;  /* no external trigger for free-run */
    /* Optional: priority/start mode if you need to adjust:
       gCfg.queueRequest[0].requestSlotPrio      = IfxEvadc_RequestSlotPriority_high;
       gCfg.queueRequest[0].requestSlotStartMode = IfxEvadc_RequestSlotStartMode_cancelInject; */

    IfxEvadc_Adc_initGroup(&s_eva.g8, &gCfg);

    /* 4) Initialize the four channels on Group 8 and map to distinct result registers */
    for (int i = 0; i < 4; ++i)
    {
        IfxEvadc_Adc_ChannelConfig cCfg;
        IfxEvadc_Adc_initChannelConfig(&cCfg, &s_eva.g8);
        cCfg.channelId      = kChIds[i];
        cCfg.resultRegister = (IfxEvadc_ChannelResult)i; /* RES0..RES3 */

        /* Optional: input class/sample time if you have higher source impedance:
           cCfg.inputClass = IfxEvadc_InputClass_group0; (then adjust gCfg.inputClass[...] before initGroup) */

        IfxEvadc_Adc_initChannel(&s_eva.ch[i], &cCfg);
    }

    /* 5) Safely fill queue0 with refill enabled
       Disable the gate while filling to keep deterministic ordering, then restore it. */
    IfxEvadc_GatingMode  savedGate   = IfxEvadc_getQueueSlotGatingMode(s_eva.g8.group, kReqSrc);
    IfxEvadc_GatingSource savedSrc   = IfxEvadc_getQueueSlotGatingSource(s_eva.g8.group, kReqSrc);
    IfxEvadc_setQueueSlotGatingConfig(s_eva.g8.group, savedSrc, IfxEvadc_GatingMode_disabled, kReqSrc);

    /* Add channels to queue0 with IFXEVADC_QUEUE_REFILL so it runs continuously */
    IfxEvadc_Adc_addToQueue(&s_eva.ch[0], kReqSrc, IFXEVADC_QUEUE_REFILL);
    IfxEvadc_Adc_addToQueue(&s_eva.ch[1], kReqSrc, IFXEVADC_QUEUE_REFILL);
    IfxEvadc_Adc_addToQueue(&s_eva.ch[2], kReqSrc, IFXEVADC_QUEUE_REFILL);
    IfxEvadc_Adc_addToQueue(&s_eva.ch[3], kReqSrc, IFXEVADC_QUEUE_REFILL);

    /* Restore gate to previous configuration (typically “always”) */
    IfxEvadc_setQueueSlotGatingConfig(s_eva.g8.group, savedSrc, savedGate, kReqSrc);

    /* 6) Start queue0 (software trigger). With refill, the sequence loops forever. */
    IfxEvadc_Adc_startQueue(&s_eva.g8, kReqSrc);
}

/* Read the latest raw result for a given logical index (0..3 in the order above).
   Returns TRUE if valid at the time of read. */
boolean evadc_g8_get_latest_raw(int idx, uint16* outRaw)
{
    if ((outRaw == NULL) || (idx < 0) || (idx > 3))
        return FALSE;

    Ifx_EVADC_G_RES r = IfxEvadc_Adc_getResult(&s_eva.ch[idx]);
    *outRaw = (uint16)r.B.RESULT;
    return (r.B.VF != 0);
}

//Read function for each EVADC channel to match the onecalib library
uint16 AnalogRead(int channel_number)
{
    uint16 localvar=0;
    evadc_g8_get_latest_raw(channel_number, &localvar);
    return (uint16)localvar;
}

/* Read all four latest raw results; valid[] is optional (pass NULL if not needed) */
void evadc_g8_get_all_latest_raw(uint16 outRaw[4], boolean valid[4])
{
    if (outRaw == NULL)
        return;

    for (int i = 0; i < 4; ++i)
    {
        Ifx_EVADC_G_RES r = IfxEvadc_Adc_getResult(&s_eva.ch[i]);
        outRaw[i] = (uint16)r.B.RESULT;
        if (valid) valid[i] = (r.B.VF != 0);
    }
}

/* Optional: stop and clear the queue if you need to reconfigure */
void evadc_g8_stop_and_clear_queue(void)
{
    IfxEvadc_Adc_clearQueue(&s_eva.g8, kReqSrc);
}


// Convert an EVADC result to millivolts.
// Parameters:
//  - raw: the raw register value (Ifx_EVADC_G_RES r; uint16 raw = (uint16)r.B.RESULT;)
//  - vref_mv: reference voltage in millivolts (e.g., 5000 for 5.0 V, 3300 for 3.3 V)
//  - resolution_bits: typically 12
//  - left_aligned: set TRUE only if you configured left-aligned storage for the channel
static inline uint32 evadc_result_to_millivolts(uint16 raw, uint32 vref_mv, uint8 resolution_bits, boolean left_aligned)
{
    // Normalize raw to right-aligned counts
    uint32 counts;
    if (left_aligned)
    {
        // EVADC result register is 16 bits; for left-aligned N-bit results, data sits in the MSBs.
        // Shift down to get the N-bit right-aligned value.
        uint8 shift = (uint8)(16u - resolution_bits);
        counts = ((uint32)raw) >> shift;
    }
    else
    {
        counts = (uint32)raw;
    }

    // Scale to mV with rounding: mV = counts * vref_mv / (2^N - 1)
    uint32 full_scale = (1u << resolution_bits) - 1u;
    // Add half divisor for rounding
    uint32 mv = (counts * vref_mv + (full_scale / 2u)) / full_scale;
    return mv;
}
