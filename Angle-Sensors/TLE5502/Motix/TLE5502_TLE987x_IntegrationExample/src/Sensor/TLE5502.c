#include "Sensor/TLE5502.h"
#include <math.h>


/* Converts differential sine/cosine channel readings into an angle in degrees [0, 360). */
static float TLE5502_DecodeAngleDeg(uint16 sinP, uint16 sinN,
                                    uint16 cosP, uint16 cosN)
{
  /* Differential signals reject common-mode offset/noise */
  float sinDiff = (float)sinP - (float)sinN;
  float cosDiff = (float)cosP - (float)cosN;
  float angle;

  /* atan2 over full circle; sine is negated to match sensor rotation direction */
  angle = atan2f(-sinDiff, cosDiff) * TLE5502_RAD_TO_DEG;

  /* atan2 returns (-180, 180] -> normalize to [0, 360) */
  if (angle < 0.0f)
  {
    angle += 360.0f;
  }

  return angle;
}

/* Samples all TLE5502 channels and returns raw voltages plus the decoded angle. */
TLE5502_t TLE5502_GetAngleAndVoltage(void)
{
  TLE5502_t data;
  Measurement_t meas;

  /* Single ADC burst keeps all four channels time-aligned */
  data.VALID = ADC_SampleAll(&meas);

  /* Extract per-channel voltages (mV) from the raw measurement set */
  data.SIN_P_mV = TLE5502_SIN_P(meas);
  data.SIN_N_mV = TLE5502_SIN_N(meas);
  data.COS_P_mV = TLE5502_COS_P(meas);
  data.COS_N_mV = TLE5502_COS_N(meas);

  if (data.VALID)
  {
    data.ANGLE_DEG = TLE5502_DecodeAngleDeg(data.SIN_P_mV, data.SIN_N_mV,
                                            data.COS_P_mV, data.COS_N_mV);
  }
  else
  {
    data.ANGLE_DEG = 0.0f; 	// If data is garbled angle shouldn't be calculated
  }

  return data;
}