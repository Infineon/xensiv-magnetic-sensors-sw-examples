#include <Evadc/Adc/IfxEvadc_Adc.h>

uint16 AnalogRead(int channel_number);

IfxEvadc_Adc evadc;
IfxEvadc_Adc_Group adcGroup;

IfxEvadc_Adc_Config adcConfig;

void evadc_g8_init_queue_continuous(void);
