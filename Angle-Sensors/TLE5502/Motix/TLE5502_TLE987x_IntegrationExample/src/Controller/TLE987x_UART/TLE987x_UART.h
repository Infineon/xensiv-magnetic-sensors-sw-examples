#ifndef SRC_CONTROLLER_TLE987X_UART_TLE987X_UART_H_
#define SRC_CONTROLLER_TLE987X_UART_TLE987X_UART_H_


#include "tle_device.h"
#include <stdio.h>


/* Prints one formatted data line (channel voltages + decoded angle) over UART. */
void TLE987x_UART_SendVoltageAngleInfo(float sinP,
                                float sinN,
                                float cosP,
                                float cosN,
                                float angle_deg)
{
    /* Positive channels first, then negative ones, so the columns pair up on screen */
    printf("| SinP:%6.3fV | CosP:%6.3fV | "
           "SinN:%6.3fV | CosN:%6.3fV | "
           "ANGLE:%7.2f [deg] |\n",
           sinP, cosP, sinN, cosN, angle_deg);
}


#endif /* SRC_CONTROLLER_TLE987X_UART_TLE987X_UART_H_ */