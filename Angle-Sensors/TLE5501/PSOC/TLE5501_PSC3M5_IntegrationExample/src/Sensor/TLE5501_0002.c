#include "TLE5501_0002.h"
#include "math.h"


TLE5501_t sensor;


void TLE5501_GetAngle(TLE5501_t* sensor)
{
	// Intermediate results from ADC result interpretation
	uint8_t meas_channel;
	int16_t meas_result;
	
	// ADC_result_buffer should only have two elements, one for each pseudo-differential channel
	for(uint8_t i = 0; i < 2; i++)
	{
		meas_channel = (uint8_t) _FLD2VAL(CY_HPPASS_FIFO_RD_DATA_CHAN_ID, ADC_result_buffer[i]);		
		meas_result  = (int16_t) _FLD2VAL(CY_HPPASS_FIFO_RD_DATA_RESULT , ADC_result_buffer[i]);
	
		switch(meas_channel)
		{
			case SIN_P_CHAN_IDX:
				sensor->SIN_DIFF_ADC = meas_result;
				break;
				
			case COS_P_CHAN_IDX:
				sensor->COS_DIFF_ADC = meas_result;
				break;
				
			default:
				break;
		}	
	}
	
	// Convert ADC readings to radians
	sensor->SIN_RAD = (sensor->SIN_DIFF_ADC * M_PI) / 180;
	sensor->COS_RAD = (sensor->COS_DIFF_ADC * M_PI) / 180;
	
	// Calculate angle (atan2 returns angle in radians)
	double angle_rad = atan2(sensor->SIN_RAD, sensor->COS_RAD);
	
	// Convert angle to degrees and add 180 to shift from [-180:180] to [0:360]
	sensor->ANGLE = (angle_rad * 180.0) / M_PI + 180;
}