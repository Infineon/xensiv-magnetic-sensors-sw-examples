#ifndef SRC_SENSOR_TLE5501_0002_H_
#define SRC_SENSOR_TLE5501_0002_H_


#include "TLE5501_0002_defines.h"
#include "src/MCU/ADC/ADC.h"
#include "src/MCU/DMA/DMA.h"
#include "cybsp.h"


// Extract ADC conversion results and calculate angle.
void TLE5501_GetAngle(TLE5501_t* sensor);


#endif /* SRC_SENSOR_TLE5501_0002_H_ */
