//*****************************************************************************
//
//! @file main.c
//!
//! @brief Template example for building apps. 
//!
//! Use makefile in gcc directory (or use it as a reference to create your own)
//! cd gcc
//! make
//! make clean (clean up)
//! make bootload (flash to device)
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "inc1.h"   // These two includes are just an example to show how to include your own files
#include "inc2.h"

static int boardSetup(void);

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    boardSetup();

    am_util_stdio_terminal_clear();

    am_util_stdio_printf("SparkFun Edge Project Template\n");
    am_util_stdio_printf("Compiled on %s, %s\n\n", __DATE__, __TIME__);
    am_util_stdio_printf("SparkFun Edge Debug Output (UART)\r\n");
    am_bsp_uart_string_print("Hello, World!\r\n");  // Sting_print has less overhead than printf (and less risky behavior since no varargs)
    am_bsp_uart_string_print("This project is meant to serve as a template for making your own apps as a makefile project\r\n");  

    uint32_t pin14Val = 0; // Default to 0 to illustrate pull-up on hardware
    am_hal_gpio_state_read( AM_BSP_GPIO_14, AM_HAL_GPIO_INPUT_READ, &pin14Val);
    am_util_stdio_printf("Value on button 14 is: %d\r\n", pin14Val);

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

    // Setup LED's as outputs
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_RED, g_AM_HAL_GPIO_OUTPUT_12);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_BLUE, g_AM_HAL_GPIO_OUTPUT_12);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_GREEN, g_AM_HAL_GPIO_OUTPUT_12);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_LED_YELLOW, g_AM_HAL_GPIO_OUTPUT_12);

    // Set up button 14 as input (has pullup resistor on hardware)
    am_hal_gpio_pinconfig(AM_BSP_GPIO_14, g_AM_HAL_GPIO_INPUT);

    // Turn on the LEDs
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_RED);
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_BLUE);
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_GREEN);
    am_hal_gpio_output_set(AM_BSP_GPIO_LED_YELLOW);

    return 0;
}