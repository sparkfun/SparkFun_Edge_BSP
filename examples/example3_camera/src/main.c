//*****************************************************************************
//
//! @file main.c
//!
//! @brief Example that takes photos with the OV7670 camera
//!
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "edge_camera.h"


// volatile uint8_t ready = 0;

static int  boardSetup(void);
static void boardTeardown(void);

void
am_gpio_isr(void)
{
    uint64_t ui64IntMask = 0x00;

    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_gpio_interrupt_status_get(false, &ui64IntMask))
    {
        am_util_stdio_printf("Error reading interrupt status\n");
    }

    //
    // Clear the GPIO interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_gpio_interrupt_clear(ui64IntMask))
    {
        am_util_stdio_printf("Error clearing interrupt status\n");
    }

    // Add your interrupt hooks here!
    edge_cam_isr( ui64IntMask );
}

// void am_iomaster0_isr(void){
    

//     uint32_t ui32IntMask = 0x00;
//     am_hal_iom_interrupt_status_get(clkbufiomHandle, false, &ui32IntMask);
//     am_hal_iom_interrupt_clear(clkbufiomHandle, ui32IntMask);

//     // am_util_stdio_printf("IOM0 ISR! 0x%x\n", ui32IntMask);

//     if(ui32IntMask & AM_HAL_IOM_INT_DCMP){
//         // am_bsp_uart_string_print("DMA\n");
//         edge_cam_start_clkbuf_xfer();
//     }

//     // edge_cam_start_clkbuf_xfer();
// }


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
    am_util_stdio_printf("SparkFun Tensorflow Debug Output (UART)\r\n");
    am_bsp_uart_string_print("Hello, UART!\r\n");

    // camResult
    am_util_stdio_printf("camera init result: %x\r\n", cameraSetup());

    /*
    * Read samples in polling mode (no int)
    */
    while(1)
    {
        // Use Button 14 to break the loop and shut down
        uint32_t pin14Val = 1; 
        am_hal_gpio_state_read( AM_BSP_GPIO_14, AM_HAL_GPIO_INPUT_READ, &pin14Val);
        if( pin14Val == 0 ){ break; }


        // am_hal_clkgen_status_t rxClkStat;
        // am_hal_clkgen_status_get(&rxClkStat);
        // am_util_stdio_printf("Clkgen status: SysclkFreq - %d, eRTCOSC - %d, xtalfail %d\r\n", rxClkStat.ui32SysclkFreq, rxClkStat.eRTCOSC, rxClkStat.bXtalFailure);

    }

    // Turn off leds
    boardTeardown();

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

static void boardTeardown(void)
{
    // Lights out
    am_hal_gpio_output_clear(AM_BSP_GPIO_LED_RED);
    am_hal_gpio_output_clear(AM_BSP_GPIO_LED_BLUE);
    am_hal_gpio_output_clear(AM_BSP_GPIO_LED_GREEN);
    am_hal_gpio_output_clear(AM_BSP_GPIO_LED_YELLOW);
}