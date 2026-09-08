#ifndef SRC_SENSOR_TLE5501_0002_DEFINES_H_
#define SRC_SENSOR_TLE5501_0002_DEFINES_H_


#include "stdint.h"
#include <stdint.h>


// Sensor struct used to group sensor information.
typedef struct
{
	int16_t SIN_DIFF_ADC;
	int16_t COS_DIFF_ADC;
	
	double SIN_RAD;
	double COS_RAD;
	
	double ANGLE;
} TLE5501_t;

extern TLE5501_t sensor;


#endif /* SRC_SENSOR_TLE5501_0002_DEFINES_H_ */
