/*******************************************************************************
* File Name:   main.c
*
* Description: This code example demonstrates how to measure the rotation and
* 			   speed of wheel using the TLE4922 Speed sensor and PSoC6 MCU.
*
* Related Document: See README.md
*
*
********************************************************************************
* Copyright 2019-2024, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define DATA_PIN P10_1
#define WHEEL_PITCH 0.003 //for example: wheel pitch in meters
#define WHEEL_TEETH 60

/*******************************************************************************
* Global Variables
*******************************************************************************/
volatile bool new_pulse = true;
volatile bool wheel_stopped = false;

volatile uint32_t last_time = 0;
volatile uint32_t current_time = 0;
volatile uint32_t time_diff = 0;
volatile uint32_t stop_time = 0;

cyhal_timer_t timer_obj;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  uint32_t status - status indicates success or failure
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_event_t (unused)
*
*******************************************************************************/
void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event)
{
	current_time = cyhal_timer_read(&timer_obj);

	if (last_time != 0)
	{	time_diff = current_time - last_time;
		new_pulse = true;
	}
	last_time = current_time;
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  System entrance point. This function configures and initializes the GPIO and
*  GPIO interrupt for the sensor. Initializes the timer, registers the ISR and
*  timer callback.
*
* Return: int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    handle_error(result);

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
    handle_error(result);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("**************** PSoC6: Tooth Wheel Speed Measurement ****************\r\n");

    /* Initialize the timer*/
    const cyhal_timer_cfg_t timer_cfg = {
    		.compare_value = 0,
			.period = 0xFFFFFFFF,
			.direction = CYHAL_TIMER_DIR_UP,
			.is_compare = false,
			.is_continuous = true,
			.value = 0
    };

    result = cyhal_timer_init(&timer_obj, NC, NULL);
    handle_error(result);

    result = cyhal_timer_configure(&timer_obj, &timer_cfg);
    handle_error(result);

    result = cyhal_timer_set_frequency(&timer_obj, 1000000); //set timer frequency to 1MHz
    handle_error(result);

    result = cyhal_timer_start(&timer_obj);
    handle_error(result);

    /* Initialize the GPIO pin for the tooth wheel pulse (P10_1) */
    result = cyhal_gpio_init(DATA_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
    handle_error(result);

    /* register the interrupt handler for the GPIO pin */
    cyhal_gpio_callback_data_t gpio_btn_callback_data;
    gpio_btn_callback_data.callback = gpio_interrupt_handler;

    cyhal_gpio_register_callback(DATA_PIN,&gpio_btn_callback_data);
    cyhal_gpio_enable_event(DATA_PIN, CYHAL_GPIO_IRQ_RISE, 7, true);

    uint32_t total_time = 0;
    uint32_t pulse_count = 0;
    float total_distance = 0.0;

    for (;;)
    {
    	if (new_pulse)
    	{
    		new_pulse = false;
    		float speed_mps = WHEEL_PITCH / (time_diff / 1000000.0);	//Speed in meters per second
    		float speed_kmph = speed_mps * 3.6; //Converting to kilometers per hour
    		printf ("Time difference: %lu us, Speed: %f km/h \r\n", time_diff, speed_kmph);

    		total_time += time_diff;
    		pulse_count++;
    		total_distance += WHEEL_PITCH;

    		stop_time = cyhal_timer_read(&timer_obj);
    		wheel_stopped = false;
    	}

    	if (!wheel_stopped && (cyhal_timer_read(&timer_obj) - stop_time)	> 2000000)	//2 seconds delay
    	{
    		printf ("wheel stopped \r\n");
    	  	wheel_stopped = true;

    	  	// Calculate average speed and RPM
    	  	if (pulse_count >0)	//Ensure there were pulses to calculate
    	  	{
    	  		float average_speed_mps = total_distance / (total_time/1000000.0);	//Average speed in meters per second
    	  		float average_speed_kmph = average_speed_mps *3.6;	//Converting to kilometers per hour
    	  		float rpm = (60.0 / (total_time/1000000.0))* WHEEL_TEETH;	//RPM
    	  		printf("Average speed: %f km/h, RPM: %f \r\n", average_speed_kmph, rpm);
    	  	}
    	  	total_time = 0;
    	  	pulse_count = 0;
    	  	total_distance = 0.0;
    	}
    }
}

/* [] END OF FILE */
