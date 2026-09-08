#include "TLE987x_ADC.h"

/* Configures ADC1 to sample the four sin/cos inputs (P2.0, P2.2, P2.4, P2.5). */
void ADC_Init_SinCos(void)
{
  ADC1_Power_On();
  ADC1_ANON_Set((uint32)ADC1_ANON_NORMAL);           /* analog front-end always-on mode */

  while (ADC1_ANON_Sts() != ADC1_ANON_NORMAL)
  {
    /* wait for the analog part to reach normal mode */
  }

  ADC1_DIVA_Set(1u);                                 /* analog clock divider */

  /* All four sin/cos channels in ONE sequence => one uninterrupted sweep. */
  ADC1_Sequence0_Set(ADC1_MASK_P20 | ADC1_MASK_P22 |
                     ADC1_MASK_P24 | ADC1_MASK_P25);

  /* Identical sample time on every channel keeps the four readings comparable */
  ADC1_Ch0_Sample_Time_Set(ADC_SAMPLE_TICKS);
  ADC1_Ch2_Sample_Time_Set(ADC_SAMPLE_TICKS);
  ADC1_Ch4_Sample_Time_Set(ADC_SAMPLE_TICKS);
  ADC1_Ch5_Sample_Time_Set(ADC_SAMPLE_TICKS);

  /* 10-bit resolution on all channels */
  ADC1_Ch0_DataWidth_10bit_Set();
  ADC1_Ch2_DataWidth_10bit_Set();
  ADC1_Ch4_DataWidth_10bit_Set();
  ADC1_Ch5_DataWidth_10bit_Set();

  /* Wait-For-Read: a valid result is not overwritten until it is read.
     In Overwrite mode a fast sequencer could replace CH0 while CH5 is still
     being fetched - a torn read that corrupts atan2. */
  ADC1_Ch0_WaitForRead_Set();
  ADC1_Ch2_WaitForRead_Set();
  ADC1_Ch4_WaitForRead_Set();
  ADC1_Ch5_WaitForRead_Set();

  ADC1_Sequencer_Mode_Sel();                         /* start sequencer-driven conversions */
}

/* Reads one full set of sin/cos samples; returns false if any result was invalid. */
bool ADC_SampleAll(Measurement_t *meas)
{
  bool ok;

  /* Each call reads the result and its valid flag exactly once. */
  ok  = ADC1_GetChResult_mV(&meas->P2_0_mV, (uint8)ADC1_CH0);
  ok &= ADC1_GetChResult_mV(&meas->P2_2_mV, (uint8)ADC1_CH2);
  ok &= ADC1_GetChResult_mV(&meas->P2_5_mV, (uint8)ADC1_CH5);
  ok &= ADC1_GetChResult_mV(&meas->P2_4_mV, (uint8)ADC1_CH4);

  return ok;                                         /* all four must be valid */
}