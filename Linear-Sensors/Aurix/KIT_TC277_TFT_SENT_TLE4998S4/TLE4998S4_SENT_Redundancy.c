/**********************************************************************************************************************
 * \file TLE4998S4_SENT_Redundancy.c
 * \copyright Copyright (C) Infineon Technologies AG 2019
 * 
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of 
 * business you are acting and (ii) Infineon Technologies AG or its licensees. If and as long as no such terms of use
 * are agreed, use of this file is subject to following:
 * 
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and 
 * accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute,
 * and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the
 * Software is furnished to do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including the above license grant, this restriction
 * and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all 
 * derivative works of the Software, unless such copies or derivative works are solely in the form of 
 * machine-executable object code generated by a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE 
 * COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN 
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 *********************************************************************************************************************/


/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/
#include <math.h>
#include "TLE4998S4_SENT_Redundancy.h"
#include "IfxSent_Sent.h"
#include "IfxSent.h"
#include "IfxGtm_Tom_Pwm.h"
#include "IfxGtm_Tim_In.h"
#include "IfxGtm_Trig.h"
#include "IfxGtm_Tom.h"

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

/* Linear Sensor define */
#define SENT_CH0B_PIN_IN              IfxSent_SENT0B_P00_1_IN       /* SENT input pin                                 */
#define ISR_PRIORITY_SENT_CHANNEL0    4                             /* Interrupt Priority of sensor                   */

#define SENT_TICK_TIME                      3.0E-6       /* TLE4998S4 is by default configured with 3 us        */
#define TLE4998_FRAME_LENGTH                6

#define NUMBER_OF_NIBBLES_FOR_CRC           7            /* Total number of nibbles used for CRC calculation    */
#define TLE4998_FRAME_CRC_SEED_VAL          0x05         /* the seed value of the fast CRC                      */

/* LED on Application Kit */
#define LED_D106_ALARM                      &MODULE_P33, 6   /* LED D106: Port, Pin definition                       */
#define LED_D109_ALARM                      &MODULE_P33, 9   /* LED D109: Port, Pin definition                       */
/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/
DataTle4998 g_dataTle4998;                          /* global variable for sent struct                               */
static SentErrorCounters g_errorCounters = {0};     /* error count                                                   */
static boolean g_alarmFlag = FALSE;                 /* alarm flag                                                    */

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/
void interruptHandlerSENT(DataTle4998 *sent);
void initSentCh0bSentMode(void);
void initSENTmoduleForTle4998(void);
void crcCalculation(uint8 channelId);
uint8 calculateCrcTle4998(uint8 *message, uint8 length);

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/
IFX_INTERRUPT(channel0SENTisr, 0, ISR_PRIORITY_SENT_CHANNEL0);

/* SENT CH0 interrupt */
void channel0SENTisr(void)
{
    interruptHandlerSENT(&g_dataTle4998);
}

/* Handle SENT interrupt */
void interruptHandlerSENT(DataTle4998 *sent)
{
    IfxSent_Sent_Channel *channel = &sent->sentChannel;
    Ifx_SENT_CH_INTSTAT   interruptStatus = IfxSent_Sent_getAndClearInterruptStatus(channel);

    if(interruptStatus.U)
    {
        /* check for error conditions */
        if (interruptStatus.U & IFXSENT_INTERRUPT_STATUS_ERROR_FLAGS)
        {
            /* Receive Buffer Overflow
             * This bit is set after a frame has been received while the old one was
             * not read from RDRx. I.e. the kernel wants to set any of the two
             * interrupts RSI and RDI and finds any of these two interrupts already
             * set. The old data is overwritten by the new data.
             */
            if (interruptStatus.B.RBI)
            {
                g_errorCounters.RBI++;
            }

            /* Transmit Buffer Underflow
             * This bit is set after data has been completely transferred (PLEN
             * exceeded) and no new data was written to SCRx.
             */
            if (interruptStatus.B.TBI)
            {
                g_errorCounters.TBI++;
            }

            /* Frequency Range Error
             * This bit is set after a Synchronization / Calibration pulse was
             * received that deviates more than +- 25% from the nominal value.
             * The referring data is ignored.
             */
            if (interruptStatus.B.FRI)
            {
                g_errorCounters.FRI++;
            }

            /* Frequency Drift Error
             * This bit is set after a subsequent Synchronization / Calibration
             * pulse was received that deviates more than 1.5625% (1/64) from its
             * predecessor.
             */
            if (interruptStatus.B.FDI)
            {
                g_errorCounters.FDI++;
            }

            /* Wrong Number of Nibbles
             * This bit is set after a more nibbles have been received than expected
             * or a Synchronization / Calibration Pulse is received too early thus
             * too few nibbles have been received
             */
            if (interruptStatus.B.NNI)
            {
                g_errorCounters.NNI++;
            }

            /* Nibbles Value out of Range
             * This bit is set after a too long or too short nibble pulse has been
             * received. I.e. value < 0 or value > 15.
             */
            if (interruptStatus.B.NVI)
            {
                g_errorCounters.NVI++;
            }

            /* CRC Error
             * This bit is set if the CRC check fails.
             */
            if (interruptStatus.B.CRCI)
            {
                g_errorCounters.CRCI++;
            }

            /* Wrong Status and Communication Nibble Error
             * In standard Serial Frame Mode (RCR.ESF is cleared), this bit is set
             * if the Status and Communication nibble shows a start bit in a frame
             * other than frame number n x 16.
             * In Extended Serial Frame Mode this bit is without function.
             */
            if (interruptStatus.B.WSI)
            {
                g_errorCounters.WSI++;
            }

            /* Serial Data CRC Error
             * This bit is set if the CRC of the serial message fails.
             * In Extended Serial Message Format, this includes a check of the Serial
             * Communication Nibble for correct 0 values of bit 3 in frames 7, 13 and 18.
             */
            if (interruptStatus.B.SCRI)
            {
                g_errorCounters.SCRI++;
            }

            /* Watch Dog Error
             * This bit is set if the Watch Dog Timer of the channel expires.
             */
            if (interruptStatus.B.WDI)
            {
                g_errorCounters.WDI++;
            }
        }

        /* transaction events */

        /* Receive Data
         * RDI is activated when a received frame is moved to a Receive Data
         * Register RDR. Both RDI and RSI will be issued together in normal use
         * cases where the frame size is not bigger than 8 nibbles and CRC is
         * correct or not checked (if RCRx.CDIS is cleared).
         */
        if (interruptStatus.B.RDI)
        {
            /* Ignore RDI bit, useful only when Frame Length is greater than
             * 8 nibbles since it can indicate that end of frame
             */
        }

        /* Receive Success
         * break point
         * This bit is set at the successfully received end of a frame.
         * Depending on bit RCRx.CDIS this indicates a successful check of the CRC.
         */
        if (interruptStatus.B.RSI)
        {
            /* decode incoming frame */
            IfxSent_Sent_Frame frame;

            /* read sent channel serial data frame of */
            IfxSent_Sent_readChannelSerialDataFrame(channel, &frame);
            /* update the sensor sent raw data */
            sent->sentRawData.U      = frame.data;
            sent->sentStatus         = frame.statusNibble;
            /* sent payload */
            sent->OUT16              = frame.data;
            sent->TEMP8              = frame.data >> 16;
            /* crc Calculation */
            crcCalculation(sent->sentChannel.channelId);
            /* increment counter */
            ++sent->interruptCounter;
            ++sent->interruptCounterChannel;
        }

        /* Transfer Data
         * This bit is set after the trigger condition was detected. Data to be
         * transferred has been moved internally. Thus a new value can be written
         * to SCRx. This can be used for back to back transfers.
         */
        if (interruptStatus.B.TDI)
        {
        }

        /* Serial Data Received
         * This bit is set after all serial data bits have been received via the
         * Status and Communication nibble. Depending on bit RCRx.SCDIS this
         * indicates a successful check of the CRC.
         */
        if (interruptStatus.B.SDI)
        {
             IfxSent_Sent_SerialMessageFrame g_serialMessage;
             /* decode incoming message */
             IfxSent_Sent_readChannelSerialMessageFrame(channel, &g_serialMessage);
        }
     }
 }

/* this function compare the CRC received and calculated */
void crcCalculation(uint8 channelId)
{
    uint8 receivedData[NUMBER_OF_NIBBLES_FOR_CRC] = {0, 0, 0, 0, 0, 0, 0};
    /* read received CRC value from sensor */
    g_dataTle4998.sentCrcReceived = IfxSent_readReceivedCrc(g_dataTle4998.sentModule.sent, channelId);

    /* swap data as sent by tle4998 sensor and add status because it is included in CRC calculation on sensor side*/
    receivedData[0] = g_dataTle4998.sentStatus;
    receivedData[1] = g_dataTle4998.sentRawData.B.nibble3;
    receivedData[2] = g_dataTle4998.sentRawData.B.nibble2;
    receivedData[3] = g_dataTle4998.sentRawData.B.nibble1;
    receivedData[4] = g_dataTle4998.sentRawData.B.nibble0;
    receivedData[5] = g_dataTle4998.sentRawData.B.nibble5;
    receivedData[6] = g_dataTle4998.sentRawData.B.nibble4;

    /* calculate CRC on the received data */
    g_dataTle4998.sentCrcCalculated = calculateCrcTle4998(&receivedData[0], NUMBER_OF_NIBBLES_FOR_CRC);
}

/*
 * This function does the Cyclic Redundancy Check(CRC) calculation for the decoded SENT nibbles.
 * Returns the calculated CRC value.
 * This CRC calculation is taken from TLE4998 sensor user manual
 */
uint8 calculateCrcTle4998(uint8 *message, uint8 length)
{
    uint8 CheckSum = TLE4998_FRAME_CRC_SEED_VAL;
    char CrcLookup[16] = {0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5};
    uint8 crcDatalen = TLE4998_FRAME_LENGTH + 1;
    for(uint8 bitdata = 0; bitdata < crcDatalen; bitdata++)
    {
        CheckSum = CheckSum ^ message[bitdata];
        CheckSum = CrcLookup[CheckSum];
    }
    return CheckSum;
}

/*
 * Initialization of SENT channel 4 for sensor
 */
float32 g_ut = 3.21;
void initSentCh0bSentMode(void)
{
    /* create module config */
    IfxSent_Sent_Config sentConfig;

    IfxSent_Sent_initModuleConfig(&sentConfig, SENT_CH0B_PIN_IN.module);

    /* initialize module */
    IfxSent_Sent_initModule(&g_dataTle4998.sentModule, &sentConfig);

    /* create channel config */
    IfxSent_Sent_ChannelConfig sentChannelConfig;
    IfxSent_Sent_initChannelConfig(&sentChannelConfig, &g_dataTle4998.sentModule);

    /* define tUnit of the external sensor */
    sentChannelConfig.tUnit = SENT_TICK_TIME;
    /* CRC is disabled, The CPU must perform the CRC on the current data by SW */

    sentChannelConfig.receiveControl.endPulseIgnored = FALSE; // no end pause pulse
    sentChannelConfig.receiveControl.alternateCrcSelected = TRUE; // TLE4998C
    sentChannelConfig.receiveControl.statusNibbleEnabled = TRUE; // Status Nibble Included in CRC
    sentChannelConfig.receiveControl.serialDataProcessingEnabled = FALSE;
    sentChannelConfig.receiveControl.serialDataDisabledCrcDisabled = TRUE;
    sentChannelConfig.receiveControl.crcModeDisabled = FALSE;
    sentChannelConfig.receiveControl.crcMethodDisabled = TRUE;
// frameCheckMode
    sentChannelConfig.receiveControl.frameLength = TLE4998_FRAME_LENGTH;
    sentChannelConfig.receiveControl.extendedSerialFrameMode = FALSE;
    sentChannelConfig.receiveControl.driftErrorsDisabled = TRUE;

    const IfxSent_Sent_Pins sentPins =
    {
        &SENT_CH0B_PIN_IN, IfxPort_InputMode_noPullDevice,  /* SENT input */
        NULL_PTR,     IfxPort_OutputMode_openDrain, /* SENT output */
        IfxPort_PadDriver_cmosAutomotiveSpeed1
    };
    /* Assign pins */
    sentChannelConfig.pins = &sentPins;
    sentChannelConfig.channelId = SENT_CH0B_PIN_IN.channelId;

    /* SPC mode off */
    sentChannelConfig.spcModeOn = FALSE;

    /* ISR priorities and interrupt target */
    sentChannelConfig.interrupt.priority = ISR_PRIORITY_SENT_CHANNEL0;
    sentChannelConfig.interrupt.isrProvider = IfxSrc_Tos_cpu0;
    sentChannelConfig.enabledInterrupts.ALL = 0x3FFF;

    /* The TLE4998 provides the nibbles in Big-endian order, Swap the nibbles 0 and 2 */
    sentChannelConfig.nibbleControl.nibblePointer0 = IfxSent_Nibble_3;
    sentChannelConfig.nibbleControl.nibblePointer1 = IfxSent_Nibble_2;
    sentChannelConfig.nibbleControl.nibblePointer2 = IfxSent_Nibble_1;
    sentChannelConfig.nibbleControl.nibblePointer3 = IfxSent_Nibble_0;
    sentChannelConfig.nibbleControl.nibblePointer4 = IfxSent_Nibble_5;
    sentChannelConfig.nibbleControl.nibblePointer5 = IfxSent_Nibble_4;

    /* interrupt requested node */
    sentChannelConfig.interuptNodeControl.errorInterruptNode                   = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.receiveBufferOverflowInterruptNode   = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.receiveDataInterruptNode             = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.receiveSuccessInterruptNode          = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.serialDataReceiveInterruptNode       = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.transferBufferUnderflowInterruptNode = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.transferDataInterruptNode            = IfxSent_InterruptNodePointer_0;
    sentChannelConfig.interuptNodeControl.watchdogErrorInterruptNode           = IfxSent_InterruptNodePointer_0;

    /* initialize channel */
    IfxSent_Sent_initChannel(&g_dataTle4998.sentChannel, &sentChannelConfig);

    g_ut = IfxSent_getChannelUnitTime(g_dataTle4998.sentModule.sent, g_dataTle4998.sentChannel.channelId);
    sentChannelConfig.tUnit = g_ut;
}

/* SENT initialization
 * This function initializes the SENT module
 */
void initSENTmoduleForTle4998()
{
    /* init SENT for sensor1 */
    initSentCh0bSentMode();
}

/*
 * This function initializes the port pin which drives the LED
 */
void initLED_ALARM(void)
{
    /* Initialization of the LED used in this example */
    IfxPort_setPinModeOutput(LED_D106_ALARM, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Switch OFF the LED (high-level active) */
    IfxPort_setPinHigh(LED_D106_ALARM);

    /* Initialization of the LED used in this example */
    IfxPort_setPinModeOutput(LED_D109_ALARM, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Switch OFF the LED (high-level active) */
    IfxPort_setPinHigh(LED_D109_ALARM);
}

/*
 * Init SENT module
 */
void initModules(void)
{
    /* Initialize the LED port pin */
    initLED_ALARM();

    /* Initial SENT module for TLE4998 */
    initSENTmoduleForTle4998();
}

/*
 *  Function to check Sent redundancy via two Sensor
 */
void checkTle4998SENTredundancy()
{
    static uint8 ignoreValueCount = 0;
    /* Ignore first some measurement to get accurate/sync value for both sensor */
    if (g_dataTle4998.interruptCounter < 3) {
        IfxPort_setPinLow(LED_D106_ALARM);
        return;
    }
    if(ignoreValueCount < 3)
    {
        ignoreValueCount++;
        IfxPort_setPinLow(LED_D109_ALARM);
    } else if (g_dataTle4998.sentCrcCalculated != g_dataTle4998.sentCrcReceived)
    {
        /* outside limit or CRC Mismatch */
        IfxPort_setPinHigh(LED_D109_ALARM);
        IfxPort_togglePin(LED_D106_ALARM);
        g_alarmFlag = TRUE;
    } else {
        /* with in limit or CRC match */
        IfxPort_setPinHigh(LED_D106_ALARM);
        IfxPort_togglePin(LED_D109_ALARM);
        g_alarmFlag = FALSE;
    }
}
