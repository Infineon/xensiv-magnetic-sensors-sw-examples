#ifndef TLE987X_ADC_H
#define TLE987X_ADC_H

#include "tle_device.h"
#include "adc1.h"
#include "types.h"

/** \brief Sample time ticks per channel. The TLE5502 outputs are relatively
 *  high impedance and share one S&H through the input mux; too few ticks
 *  causes channel-to-channel crosstalk. */
#define ADC_SAMPLE_TICKS  (20u)


/*******************************************************************************
 * Structure Name: Measurement_t
 *******************************************************************************
 *  \brief One coherent set of ADC1 results, in millivolts.
 *  \note Field names follow the physical port pins. adc1.h confirms the
 *  channel map is 1:1: CH0=P2.0, CH2=P2.2, CH4=P2.4, CH5=P2.5. 
 ******************************************************************************/
typedef struct Measurement
{
  uint16 P2_0_mV;
  uint16 P2_2_mV;
  uint16 P2_4_mV;
  uint16 P2_5_mV;
} Measurement_t;

/*******************************************************************************
 * Function Name: ADC_Init_SinCos
 *******************************************************************************
 *  \brief Configures ADC1 for coherent sin/cos acquisition.
 *  Places all four channels in Sequence 0 and enables Wait-For-Read so a
 *  result is held until read, guaranteeing all four values come from the
 *  same sweep. Call once after TLE_Init(). 
 ******************************************************************************/
void ADC_Init_SinCos(void);

/*******************************************************************************
 * Function Name: ADC_SampleAll
 *******************************************************************************
 *  \brief Saves one complete sample set.
 *  \param[out] meas destination, untouched fields are still written on failure
 *  \retval true  all four channels returned a fresh, valid result
 *  \retval false at least one channel had no new result - do not use for angle
 *  \note Each valid flag is read exactly once. Reading a valid flag clears it
 *  (see adc1.h), so never poll them separately before calling this. 
 ******************************************************************************/
bool ADC_SampleAll(Measurement_t *meas);



#endif /* TLE987X_ADC_H */