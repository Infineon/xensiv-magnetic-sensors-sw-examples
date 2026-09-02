#include "UART.h"

#include "cy_retarget_io.h"
#include "cycfg_peripherals.h"
#include <math.h>
#include <stdint.h>


// Data structures required for the UART PC HAL.
// Every interaction with these structures is internal, no FW assignments/reads.
static cy_stc_scb_uart_context_t    UART_PC_context;
static mtb_hal_uart_t               UART_PC_hal_obj;


void PSC3M5_UART_Init(void)
{
	cy_rslt_t result;
	
	result = (cy_rslt_t)Cy_SCB_UART_Init(UART_PC_HW, &UART_PC_config, &UART_PC_context);
	if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
	
	Cy_SCB_UART_Enable(UART_PC_HW);
	
	result = mtb_hal_uart_setup(&UART_PC_hal_obj, &UART_PC_hal_config, &UART_PC_context, NULL);
	if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
	
	result = cy_retarget_io_init(&UART_PC_hal_obj);
	if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

void PSC3M5_UART_SendAngleInfo(uint16_t angle, uint8_t slave)
{	
	// Print angle in hexadecimal
	printf("Sensor%u ->ANGLE [LSB]: 0x", slave);
	
	if(angle <= 0x0fff)
		printf("0");
	
	if(angle <= 0x00ff)
		printf("0");
	
	if(angle <= 0x000f)
		printf("0");
	
	printf("%X | ", angle);
	
	// Print angle in degrees
	double angle_deg = ((uint32_t)angle * 360.0) / 65535; 

	printf("Sensor%u ->ANGLE [deg]: %.3f\r\n", slave, angle_deg);	
}

void PSC3M5_UART_SendSyncAngleInfo(uint16_t angle, uint8_t slave)
{	
	// Print angle in hexadecimal
	printf("Sensor%u ->ANGLE [LSB]: 0x", slave);
	printf("%04X | ", angle);
	
	// Print angle in degrees
	double angle_deg = ((uint32_t)angle * 360.0) / 16384; // 14 bit value 

	printf("Sensor%u ->ANGLE [deg]: %.3f\r\n", slave, angle_deg);	
}



