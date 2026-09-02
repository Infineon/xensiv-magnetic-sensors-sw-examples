#include "UART.h"

#include "cy_retarget_io.h"
#include "cycfg_peripherals.h"
#include <math.h>


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

void PSC3M5_UART_SendAngleInfo(TLE5501_t* sensor)
{	
	// Print SIN information
	printf("SIN [rad]: ");
	if(sensor->SIN_RAD > 0)	
	{
		printf("+");
	} 
	else 
	{
		printf("-");
	}		
	printf("%.3f  |  ", fabs(sensor->SIN_RAD));
	
	// Print COS information
	printf("COS [rad]: ");
	if(sensor->COS_RAD > 0)	
	{
		printf("+");
	} 
	else
	{
		printf("-");
	}
	printf("%.3f  |  ", fabs(sensor->COS_RAD));
	
	// Print ANGLE information
	double abs_angle = fabs(sensor->ANGLE);
	
	printf("ANGLE [deg]: ");
	if(abs_angle < 100)
	{
		printf(" ");		
	}
	
	if(abs_angle < 10)
	{
		printf(" ");
	}
		
	 printf("%.3f\r\n", abs_angle);	
}


