//*****************************************************************************
//
//! @file blank_slate.c
//!
//! @brief Example that turns LEDs on, prints debug message, and waits
//!
//! SWO is configured in 2M baud, 8-n-1 mode.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "tf_adc.h"
#include "tf_accelerometer.h"

static int boardSetup(void);
static int testADC(void);

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    boardSetup();

    am_util_stdio_terminal_clear();

    am_util_stdio_printf("SparkFun Edge Board Test\n");
    am_util_stdio_printf("Compiled on %s, %s\n\n", __DATE__, __TIME__);
    am_util_stdio_printf("SparkFun Tensorflow Debug Output (SWO)\r\n");
    am_bsp_uart_string_print("Hello, UART!\r\n");

    initAccelerometer();

    testADC();
    

    while (1) 
    {
    }

    // Disable debug
    am_bsp_debug_printf_disable();
    
    // Go to Deep Sleep.
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
}

static int boardSetup(void)
{
    // Set the clock frequency.
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    // Set the default cache configuration
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    // Configure the board for low power operation.
    am_bsp_low_power_init();

    // Initialize the printf interface for ITM/SWO output.
    am_bsp_uart_printf_enable(); // Enable UART - will set debug output to UART
    //am_bsp_itm_printf_enable(); // Redirect debug output to SWO

    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_RED, g_AM_HAL_GPIO_OUTPUT_12);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_BLUE, g_AM_HAL_GPIO_OUTPUT_12);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_GREEN, g_AM_HAL_GPIO_OUTPUT_12);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_YELLOW, g_AM_HAL_GPIO_OUTPUT_12);

    am_hal_gpio_output_set(AM_BSP_GPIO_LED_RED);
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_BLUE);
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_GREEN);
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_YELLOW);

    return 0;
}

static int testADC(void)
{
    initADC();
    enableAdcInterrupts();
    //triggerAdc();

    return 0;
}