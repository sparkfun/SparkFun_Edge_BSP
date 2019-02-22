//*****************************************************************************
//
//! @file main.c
//!
//! @brief Edge Test is an example to demonstrate the built-in hardware on the Edge board
//!
//! SWO is configured in 2M baud, 8-n-1 mode.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "tf_adc.h"
#include "tf_accelerometer.h"

static int  boardSetup(void);
static void boardTeardown(void);
static int  testADC(void);

axis3bit16_t data_raw_acceleration;
axis1bit16_t data_raw_temperature;
float acceleration_mg[3];
float temperature_degC;

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

    int accInitRes = initAccelerometer();
    am_util_stdio_printf("Accelerometer init returned %8x\r\n", accInitRes);

    testADC();
    

    /*
    * Read samples in polling mode (no int)
    */
    while(1)
    {
        // Use Button 14 to break the loop and shut down
        uint32_t pin14Val = 1; 
        am_hal_gpio_state_read( AM_BSP_GPIO_14, AM_HAL_GPIO_INPUT_READ, &pin14Val);
        if( pin14Val == 0 ){ break; }

        lis2dh12_reg_t reg;

        /*
        * Read output only if new value available
        */
        lis2dh12_xl_data_ready_get(&dev_ctx, &reg.byte);
        if (reg.byte)
        {
            /* Read accelerometer data */
            memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
            lis2dh12_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
            acceleration_mg[0] =
            lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[0]);
            acceleration_mg[1] =
            lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[1]);
            acceleration_mg[2] =
            lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[2]);
            
            // am_util_stdio_printf("Acc [mg] %04.2f x, %04.2f y, %04.2f z, Temp [deg C] %04.2f, MIC0 [counts / 2^14] %d\r\n",
            //         acceleration_mg[0], acceleration_mg[1], acceleration_mg[2], temperature_degC, (audioSample) );

            am_util_stdio_printf("%d\r\n",
                    (audioSample) );

        }

        lis2dh12_temp_data_ready_get(&dev_ctx, &reg.byte);      
        if (reg.byte)    
        {
            /* Read temperature data */
            memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
            lis2dh12_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
            temperature_degC =
            lis2dh12_from_lsb_hr_to_celsius(data_raw_temperature.i16bit);
        }
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

static int testADC(void)
{
    initADC();
    enableAdcInterrupts();
    //triggerAdc();

    return 0;
}