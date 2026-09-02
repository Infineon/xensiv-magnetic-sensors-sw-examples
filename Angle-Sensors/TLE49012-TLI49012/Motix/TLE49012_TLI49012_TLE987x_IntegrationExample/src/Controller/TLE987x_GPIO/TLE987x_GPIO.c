#include "TLE987x_GPIO.h"


void GPIO_SetOutputLevelLow(volatile uint8 *port, uint8_t pin, uint8_t msk)
{
	Field_Clr8(port, msk);
}

void GPIO_SetOutputLevelHigh(volatile uint8 *port, uint8_t pin, uint8_t msk)
{
	Field_Mod8(port, pin, msk, 1u);
}

void GPIO_TogglePin(volatile uint8 *port, uint8_t pin, uint8_t msk)
{
	Field_Inv8(port, msk);
}