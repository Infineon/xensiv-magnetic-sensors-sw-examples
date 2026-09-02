#ifndef TLE5502_H
#define TLE5502_H

#include "../src/Controller/TLE987x_ADC/TLE987x_ADC.h"

//--------------------------------------------------------//
//---------------------- Structures ----------------------//
//--------------------------------------------------------//
typedef struct TLE5502
{
  uint16 SIN_P_mV;
  uint16 SIN_N_mV;
  uint16 COS_P_mV;
  uint16 COS_N_mV;
  float  ANGLE_DEG;   /* VALID is set true when data is sampled sequentially	*/
  bool   VALID;
} TLE5502_t;

//--------------------------------------------------------//
//------------------ Struct ease of use ------------------//
//--------------------------------------------------------//
#define TLE5502_SIN_P(m)  ((m).P2_0_mV)
#define TLE5502_SIN_N(m)  ((m).P2_5_mV)
#define TLE5502_COS_P(m)  ((m).P2_2_mV)
#define TLE5502_COS_N(m)  ((m).P2_4_mV)

//--------------------------------------------------------//
//--------------------- Pi definiton ---------------------//
//--------------------------------------------------------//
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//--------------------------------------------------------//
//---------------------- RAD to DEG ----------------------//
//--------------------------------------------------------//
#define TLE5502_RAD_TO_DEG  (180.0f / (float)M_PI)

/*******************************************************************************
 * Function Name: TLE5502_GetAngleAndVoltage
 *******************************************************************************
 *  \brief Samples the sensor and decodes the shaft angle.
 *  \note Always check .VALID before using .ANGLE_DEG. 
 ******************************************************************************/
TLE5502_t TLE5502_GetAngleAndVoltage(void);


#endif /* TLE5502_H */