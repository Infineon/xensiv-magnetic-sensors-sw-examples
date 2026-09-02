#include "ADC.h"
#include "cy_hppass.h"
#include "cy_syslib.h"
#include "cycfg_peripherals.h"
#include "src/MCU/DMA/DMA.h"
#include "cy_hppass_ac.h"
#include "stdio.h"
#include <stdint.h>


void PSC3M5_ADC_Init(void)
{
	// No interrupt required, since conversion trigger is provided by by FW.
	
	// Start the HPPASS peripheral.
    if(CY_HPPASS_SUCCESS != Cy_HPPASS_AC_Start(0U, 10000U))
    {
        CY_ASSERT(0);
    }
}

void PSC3M5_ADC_TriggerMeasurement(void)
{
	Cy_HPPASS_SetFwTrigger(CY_HPPASS_TRIG_0_MSK);
	
	while(!ADC_DMA_done_flag);
	
	// Reset done flag
	ADC_DMA_done_flag = false;
}