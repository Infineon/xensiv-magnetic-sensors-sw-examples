
#include "cy_pdl.h"
#include "cybsp.h"
#include <stdio.h>


#define GAIN_A 				0.0081
#define OFFSET_B 			-0.00095

#define FULLSCALE_CODE 		4096
#define VDD_IN 				4.77

cy_stc_scb_uart_context_t CYBSP_DEBUG_UART_context;

volatile char buff[50];

int main(void)
{
    /*Variables to store analog to digital conversion results*/
    volatile int sensorValue=0;
	volatile double analogVtg, pressure;
	volatile char buff[50];

    cy_rslt_t result;
	cy_en_sar_status_t sar_result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the SAR ADC with the device configurator generated structure*/
    result = Cy_SAR_Init(SAR0, &pass_0_sar_0_config);
    if (result != CY_SAR_SUCCESS)
    {
        CY_ASSERT(0);
    }


    Cy_SCB_UART_Init(CYBSP_DEBUG_UART_HW , &CYBSP_DEBUG_UART_config, &CYBSP_DEBUG_UART_context);
    Cy_SCB_UART_Enable(CYBSP_DEBUG_UART_HW);
    /* Enable the SAR ADC */
    Cy_SAR_Enable(SAR0);

    /* Print message */
    Cy_SCB_UART_PutString(CYBSP_DEBUG_UART_HW,"\x1b[2J\x1b[;H");
    Cy_SCB_UART_PutString(CYBSP_DEBUG_UART_HW,"-----------------------------------------------------------\r\n");
    Cy_SCB_UART_PutString(CYBSP_DEBUG_UART_HW,"Interfacing Xensiv KP215F1701 with CY8CKIT-149\r\n");
    Cy_SCB_UART_PutString(CYBSP_DEBUG_UART_HW,"-----------------------------------------------------------\r\n");
	
    for (;;)
    {
		for(volatile uint8_t i=0;i<10;i++)
		{
	
	        /*1. Start the Single-shot conversion */
	        Cy_SAR_StartConvert(SAR0, CY_SAR_START_CONVERT_SINGLE_SHOT);
	
	        /*2. Wait till the sample is ready */
	        sar_result = Cy_SAR_IsEndConversion(SAR0, CY_SAR_WAIT_FOR_RESULT);
	
			if(sar_result == CY_SAR_SUCCESS)
			{
			        /* Get the result from Input 0 */
					sensorValue += Cy_SAR_GetResult16(SAR0, 0);	
	        		Cy_SysLib_Delay(10);
			}

		}
		
		/*3. Convert raw average to Voltage */
		sensorValue	/=10;

		/*Convert the 12-bit digital code to analog voltage( in mV)*/
     	analogVtg = Cy_SAR_CountsTo_mVolts(SAR0, 0, sensorValue);
		
		/*milli volts to volts*/
		analogVtg/=1000;
		
		/*4. Convert Voltage to Pressure*/
		pressure = ((analogVtg/VDD_IN)-(float)OFFSET_B)/((float)GAIN_A);
		
		/*5. Print the results*/
		sprintf(buff,"Pressure  = %.3f kPa\r\n ", pressure);
    	Cy_SCB_UART_PutString(CYBSP_DEBUG_UART_HW,(const char*)&buff);

        /*6. Delay between conversions*/
        Cy_SysLib_Delay(1000);
    }
}


