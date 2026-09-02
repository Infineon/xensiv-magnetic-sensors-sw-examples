#ifndef SRC_CONTROLLER_TLE987X_UART_TLE987X_UART_H_
#define SRC_CONTROLLER_TLE987X_UART_TLE987X_UART_H_


#include "tle_device.h"
#include <stdio.h>


void TLE987x_UART_SendAngleInfo(uint16_t angle)
{	
	// Print angle in hexadecimal
	printf("ANGLE [LSB]: 0x");
	
	if(angle <= 0x0fff)
		printf("0");
	
	if(angle <= 0x00ff)
		printf("0");
	
	if(angle <= 0x000f)
		printf("0");
	
	printf("%X | ", angle);
	
	// Print angle in degrees
	double angle_deg = ((uint32_t)angle * 360.0) / 65535; 

	printf("ANGLE [deg]: %07.3f\n", angle_deg);	
}


#endif /* SRC_CONTROLLER_TLE987X_UART_TLE987X_UART_H_ */