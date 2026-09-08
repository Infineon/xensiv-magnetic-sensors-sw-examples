/*******************************************************************************
 *
* Description:
* PSoC™ 6 example for KP467 pressure sensor over SPI. Provides a UART menu
* to read pressure/temperature in 10/12/14-bit modes and demonstrates the
* Low-Power Monitoring (LPM) feature using a WAKE pin.
*
* Board:
* CY8CKIT-062-BLE (PSoC 6)
*
* Notes:
* - SPI pins (P10_0..P10_3) are configured in ModusToolbox Device Configurator:
* P10_0=MOSI, P10_1=MISO, P10_2=SCLK, P10_3=CS (SCB1)
* - WOUT pin for LPM is P10_4 (configured below).
* - Press 'm' to return to the menu from any streaming mode.
*
*******************************************************************************/


#include "cyhal.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*==============================================================================
* Macros & Command Words
*============================================================================*/
/* Assign SPI interrupt number and priority */
#define SPI_INTR_NUM ((IRQn_Type) scb_1_interrupt_IRQn)
#define SPI_INTR_PRIORITY (3U)

/* KP467 standard SPI Commands */
#define CMD_SENSOR_ID 	0xE000
#define CMD_PRESSURE_10BIT 	0x2000
#define CMD_TEMPERATURE_10BIT 	0x4000
#define CMD_PRESSURE_12BIT 	0x2400
#define CMD_TEMPERATURE_12BIT 	0x4400
#define CMD_PRESSURE_14BIT 	0x2800
#define CMD_TEMPERATURE_14BIT 	0x4800
#define CMD_DUMMY 	0x0000

/* LPM: WOUT pin (connect sensor WOUT ➜ P10_4) */
#define WAKEUP_PIN 	P10_4

/* KP467 LPM command words */
#define KP467_START_LPM 	0xA800
#define KP467_P_LPM_STAT 	0xA900
#define KP467_P_VALUE_IT_N	0xAA00
#define KP467_P_VALUE_IT_N1 	0xAB00
#define KP467_P_VALUE_IT_N2 	0xAC00
#define KP467_P_PHASE_2_READINGS 	0xAD00

/*******************************************************************************
* Global Variables
*******************************************************************************/
static cy_stc_scb_spi_context_t spiContext;
static cyhal_gpio_t wakeup_pin = WAKEUP_PIN;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void SPI_Isr(void);
uint16_t spi_send_command(uint16_t cmd);
uint8_t check_parity(uint16_t data_word);
void print_diag_bits(uint8_t diag,uint8_t num_bits);
void print_menu(void);

void read_sensor_10bit(uint32_t *counter);
void read_sensor_12bit(uint32_t *counter);
void read_sensor_14bit(uint32_t *counter);

/* Unit conversions */
float convert_pressure_10bit(uint16_t raw);
float convert_temperature_10bit(uint16_t raw);
float convert_pressure_12bit(uint16_t raw);
float convert_temperature_12bit(uint16_t raw);
float convert_pressure_14bit(uint16_t raw);
float convert_temperature_14bit(uint16_t raw);

/* LPM Demo */
void LPM_demonstration(void);

int main(void)
{
    cy_rslt_t result;
    uint32_t counter = 0;
    char user_input = 0;

#if defined (CY_DEVICE_SECURE)
    cyhal_wdt_t wdt_obj;

    /* Clear watchdog timer so that it doesn't trigger a reset */
    result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    cyhal_wdt_free(&wdt_obj);
#endif /* #if defined (CY_DEVICE_SECURE) */

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init_fc(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
            CYBSP_DEBUG_UART_CTS,CYBSP_DEBUG_UART_RTS,CY_RETARGET_IO_BAUDRATE);
    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Configure SPI to operate */
    Cy_SCB_SPI_Init(SCB1, &scb_1_config, &spiContext);

    /* Populate configuration structure (code specific for CM4) */
    const cy_stc_sysint_t spiIntrConfig =
    {
        .intrSrc      = SPI_INTR_NUM,
        .intrPriority = SPI_INTR_PRIORITY,
    };

    /* Hook interrupt service routine and enable interrupt */
    Cy_SysInt_Init(&spiIntrConfig, &SPI_Isr);
    NVIC_EnableIRQ(SPI_INTR_NUM);

    /* Enable SPI to operate */
    Cy_SCB_SPI_Enable(SCB1);

    /* Enable global interrupts */
    __enable_irq();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("****************** KP467 Sensor Interface (PSoC6) ******************\r\n\n");

    result = cyhal_gpio_init(wakeup_pin, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    for (;;)
    {

    	print_menu();
    	do { user_input = getchar(); } while (user_input == '\n' || user_input == '\r');

    	printf("Selected: %c\r\n", user_input);

    	if (user_input == '1') {
    		printf("Starting 10-bit Mode. Press 'm' to return.\r\n");
    		while (1) {
    		read_sensor_10bit(&counter);
    		Cy_SysLib_Delay(500);
    		if (cyhal_uart_readable(&cy_retarget_io_uart_obj) && getchar() == 'm') break;
    		}
    	}

    	else if (user_input == '2') {
    		printf("Starting 12-bit Mode. Press 'm' to return.\r\n");
    		while (1) {
    			read_sensor_12bit(&counter);
    			Cy_SysLib_Delay(500);
    			if (cyhal_uart_readable(&cy_retarget_io_uart_obj) && getchar() == 'm') break;
    		}
    	}

    	else if (user_input == '3') {
    		printf("Starting 14-bit Mode. Press 'm' to return.\r\n");
    		while (1) {
    			read_sensor_14bit(&counter);
    			Cy_SysLib_Delay(500);
    			if (cyhal_uart_readable(&cy_retarget_io_uart_obj) && getchar() == 'm') break;
    		}
    	}

    	else if (user_input == '4') {
    		printf("Starting LPM Demo (4-wire SPI). Press 'm' to return.\r\n");

    		LPM_demonstration();
    	}

    	else {
    		printf("Invalid option. Try again.\r\n");
    	}
    }
}

/*******************************************************************************
* Helper Functions
*******************************************************************************/
void print_menu(void)
{
printf("\r\n========== KP467 UART Menu ==========\r\n");
printf("1: Read Sensor Data - 10-bit resolution\r\n");
printf("2: Read Sensor Data - 12-bit resolution\r\n");
printf("3: Read Sensor Data - 14-bit resolution\r\n");
printf("4: LPM Demo (WAKE on P10_4)\r\n");
printf("m: Go back to menu\r\n");
printf("======================================\r\n");
}

uint16_t spi_send_command(uint16_t cmd)
{
	uint8_t tx[2] = { cmd >> 8, cmd & 0xFF };
	uint8_t rx[2] = {0};

	Cy_SCB_SPI_Transfer(SCB1, tx, rx, 2, &spiContext);
	while (CY_SCB_SPI_TRANSFER_ACTIVE & Cy_SCB_SPI_GetTransferStatus(SCB1, &spiContext));

	return (rx[0] << 8) | rx[1];
}

uint8_t check_parity(uint16_t data_word)
{
	uint8_t count = 0;
	for (uint8_t i = 0; i < 15; i++) {
		count += (data_word >> i) & 0x01;
	}
	return (count % 2);
}

void print_diag_bits (uint8_t diag, uint8_t num_bits){
	for(int8_t i = num_bits-1; i>=0; i--)
		printf("%d",(diag>>i)&0x01);
}

/* print 16-bit binary MSB->LSB */
void print_u16_binary(uint16_t v)
{
    for (int i = 15; i >= 0; --i)
    	printf("%d", (v >> i) & 0x1);
}

void print_menu_lpm_hint(void)
{
    printf("Note: LPM demo uses WAKE on P10_4. Connect sensor WAKE -> P10_4.\r\n");
}

/*******************************************************************************
* Conversion Functions - Refer to Chapter 4.2 of User Manual
*******************************************************************************/
float convert_pressure_10bit(uint16_t raw)
{
	uint16_t value = (raw >> 1) & 0x3FF;
	return ((float)value - (-297.0f)) / 6.60f;
}

float convert_temperature_10bit(uint16_t raw)
{
	uint16_t value = (raw >> 1) & 0x3FF;
	return ((float)value - 238.7f) / 6.2f;
}

float convert_pressure_12bit(uint16_t raw)
{
	uint16_t value = (raw >> 1) & 0xFFF;
	return ((float)value - (-1189.0f)) / 26.42f;
}

float convert_temperature_12bit(uint16_t raw)
{
	uint16_t value = (raw >> 1) & 0xFFF;
	return ((float)value - 992.73f) / 24.82f;
}

float convert_pressure_14bit(uint16_t raw)
{
	uint16_t value = (raw >> 1) & 0x3FFF;
	return ((float)value - (-4756.0f)) / 105.70f;
}

float convert_temperature_14bit(uint16_t raw)
{
	uint16_t value = (raw >> 1) & 0x3FFF;
	return ((float)value - 3971.64f) / 99.29f;
}

/*******************************************************************************
* Sensor Read Functions
*******************************************************************************/
void read_sensor_10bit(uint32_t *counter)
{
	spi_send_command(CMD_SENSOR_ID);
	uint16_t id = spi_send_command(CMD_PRESSURE_10BIT);
	uint16_t pressure_raw = spi_send_command(CMD_TEMPERATURE_10BIT);
	uint16_t temp_raw = spi_send_command(CMD_DUMMY); // discard

	if (check_parity(pressure_raw) && check_parity(temp_raw)) {
		float p = convert_pressure_10bit(pressure_raw);
		float t = convert_temperature_10bit(temp_raw);
		uint8_t diag = (pressure_raw >> 11) & 0x1F; // 5-bit DIAG
		printf("%lu [10bit] ID:0x%04X P_Raw:0x%04x P:%.2f kPa T_Raw:0x%04x T:%.2f C DIAG:",(*counter)++, id, pressure_raw, p, temp_raw, t);
		print_diag_bits(diag, 5);
		printf("\r\n");
	}
	else {
		printf("%lu [10bit] Parity Error\r\n", (*counter)++);
	}
}

void read_sensor_12bit(uint32_t *counter)
{
	spi_send_command(CMD_SENSOR_ID);
	uint16_t id = spi_send_command(CMD_PRESSURE_12BIT);
	uint16_t pressure_raw = spi_send_command(CMD_TEMPERATURE_12BIT);
	uint16_t temp_raw = spi_send_command(CMD_DUMMY); // discard

	if (check_parity(pressure_raw) && check_parity(temp_raw)) {
		float p = convert_pressure_12bit(pressure_raw);
		float t = convert_temperature_12bit(temp_raw);
		uint8_t diag = (pressure_raw >> 13) & 0x7; // 3-bit DIAG
		printf("%lu [12bit] ID:0x%04X P_Raw:0x%04x P:%.2f kPa T_Raw: 0x%04X T:%.2f C DIAG:",(*counter)++, id, pressure_raw, p, temp_raw,t);
		print_diag_bits(diag, 3);
		printf("\r\n");
	}
	else {
		printf("%lu [12bit] Parity Error\r\n", (*counter)++);
	}
}

void read_sensor_14bit(uint32_t *counter)
{
	spi_send_command(CMD_SENSOR_ID);
	uint16_t id = spi_send_command(CMD_PRESSURE_14BIT);
	uint16_t pressure_raw = spi_send_command(CMD_TEMPERATURE_14BIT);
	uint16_t temp_raw =spi_send_command(CMD_DUMMY); // discard

	if (check_parity(pressure_raw) && check_parity(temp_raw)) {
		float p = convert_pressure_14bit(pressure_raw);
		float t = convert_temperature_14bit(temp_raw);
		uint8_t diag = (pressure_raw >> 15) & 0x1; // 1-bit DIAG
		printf("%lu [14bit] ID:0x%04X P_Raw:0x%04x P:%.2f kPa T_Raw:0x%04x T:%.2f C DIAG:",(*counter)++, id,pressure_raw, p,temp_raw, t);
		print_diag_bits(diag, 1);
		printf("\r\n");
	}
	else {
		printf("%lu [14bit] Parity Error\r\n", (*counter)++);
	}
}

void LPM_demonstration(void)
{
    char input = 0;

    bool wake = cyhal_gpio_read(wakeup_pin);
    printf("before: %d\r\n", wake ? 1 : 0);

    /* Initial 14-bit read & print */
    uint32_t dummy_counter = 0;
    read_sensor_14bit(&dummy_counter);
    Cy_SysLib_Delay(200);

    /* Start LPM via SPI command */
    spi_send_command(KP467_START_LPM);
    printf("\r\nLPM STARTED -- 4 WIRE SPI\r\n");
    printf("WAKE-UP PIN Status (polling every 500 ms). Press 'm' to return.\r\n");

   spi_send_command(CMD_DUMMY);
   wake = cyhal_gpio_read(wakeup_pin);
   printf("after : %d\r\n", wake ? 1 : 0);

    while (input != 'm')
    {
        if (cyhal_uart_readable(&cy_retarget_io_uart_obj))
        {
            input = getchar();
            if (input == 'm') break;
        }

        /* Poll WAKE pin */
        bool wake = cyhal_gpio_read(wakeup_pin);
        printf("%d\r\n", wake ? 1 : 0);
        Cy_SysLib_Delay(500);

        if (wake)
        {
            printf("\r\n---->>> Sensor sent WAKE UP! <<<----\r\n");

            /* Request LPM registers */
            (void)spi_send_command(KP467_P_LPM_STAT);            /* prime */
            uint16_t LPM_stat          = spi_send_command(KP467_P_VALUE_IT_N);
            uint16_t p_it_n            = spi_send_command(KP467_P_VALUE_IT_N1);
            uint16_t p_it_n1           = spi_send_command(KP467_P_VALUE_IT_N2);
            uint16_t p_it_n2           = spi_send_command(KP467_P_PHASE_2_READINGS);
            uint16_t n_phase2_readings = spi_send_command(CMD_DUMMY);   /* read count */

            /* Stored values are 13-bit; shift/scale like 14-bit branch */
            float p_it_n_f  = (((p_it_n  << 1) & 0x3FFF) - (-4756.0f)) / 105.70f;
            float p_it_n1_f = (((p_it_n1 << 1) & 0x3FFF) - (-4756.0f)) / 105.70f;
            float p_it_n2_f = (((p_it_n2 << 1) & 0x3FFF) - (-4756.0f)) / 105.70f;

            float delta_p1_threshold = fabsf(p_it_n2_f - p_it_n1_f);
            float delta_p2_threshold = fabsf(p_it_n1_f - p_it_n_f);

            printf("LPM Status Register:\t");
            print_u16_binary((uint16_t)(LPM_stat & 0xFFFF));
            printf("\r\n");

            printf("P Value (it. N):\t%.2f\r\n",  p_it_n_f);
            printf("P Value (it. N-1):\t%.2f\r\n", p_it_n1_f);
            printf("P Value (it. N-2):\t%.2f\r\n", p_it_n2_f);

            /* Drop parity bit from count */
            printf("Readings Phase 2:\t%u\r\n", (unsigned)((n_phase2_readings >> 1) & 0x7FFF));

            printf("Delta_p1_threshold:\t%.2f\r\n", delta_p1_threshold);
            printf("Delta_p2_threshold:\t%.2f\r\n", delta_p2_threshold);
            printf("\r\n");

            /* Bit-level interpretation */
            printf("LPM status register interpretation:\r\n");
            printf("Gradient phase 1 hit:\t\t%u\r\n", (unsigned)((LPM_stat >> 15) & 0x1));
            printf("Gradient phase 2 hit:\t\t%u\r\n", (unsigned)((LPM_stat >> 14) & 0x1));
            printf("Uncorr. error in LPM storage:\t%u\r\n", (unsigned)((LPM_stat >> 13) & 0x1));
            printf("EEPROM unprogrammed:\t\t%u\r\n", (unsigned)((LPM_stat >> 12) & 0x1));
            printf("Uncorr. error in EEPROM:\t%u\r\n", (unsigned)((LPM_stat >> 11) & 0x1));
            printf("Error in EEPROM contr. page:\t%u\r\n", (unsigned)((LPM_stat >> 10) & 0x1));
            printf("Overvoltage detected:\t\t%u\r\n", (unsigned)((LPM_stat >> 9) & 0x1));
            printf("Pressure out of range high:\t%u\r\n", (unsigned)((LPM_stat >> 8) & 0x1));
            printf("Pressure out of range low:\t%u\r\n",  (unsigned)((LPM_stat >> 7) & 0x1));
            printf("Diag 1 error:\t\t\t%u\r\n",       (unsigned)((LPM_stat >> 6) & 0x1));
            printf("Diag 2 error:\t\t\t%u\r\n",       (unsigned)((LPM_stat >> 5) & 0x1));
            printf("TJ Diag error:\t\t\t%u\r\n",       (unsigned)((LPM_stat >> 4) & 0x1));
            printf("Full power-on reset:\t\t%u\r\n",   (unsigned)((LPM_stat >> 3) & 0x1));
            printf("Wake up via NCS + SCLK:\t\t%u\r\n", (unsigned)((LPM_stat >> 2) & 0x1));
            printf("LPM counter self test error:\t%u\r\n", (unsigned)((LPM_stat >> 1) & 0x1));

            break;
        }
    }

   (void)spi_send_command(CMD_DUMMY);
   (void)spi_send_command(CMD_DUMMY);


}

/*******************************************************************************
* SPI Interrupt Service Routine
*******************************************************************************/
void SPI_Isr(void)
{
Cy_SCB_SPI_Interrupt(SCB1, &spiContext);
}

/* [] END OF FILE */
