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

#define SPI_CLOCK_FREQ 2000000

#define DIRECTION_TX

#define RX_SIZE 256
uint8_t rx_buffer[RX_SIZE];

#define TX_SIZE 16
uint8_t tx_buffer[TX_SIZE];


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

    am_util_stdio_printf("SparkFun BlackBoard Artemis SPI Testing\n");
    am_util_stdio_printf("Compiled on %s, %s\n\n", __DATE__, __TIME__);
    am_bsp_uart_string_print("Hello, World!\r\n");  // Sting_print has less overhead than printf (and less risky behavior since no varargs)

    uint32_t retVal32 = 0;


    // Try to start up a SPI port:
    const uint8_t   iomaster_instance = 0;
    void*           iomaster_handle = NULL;

    am_hal_iom_config_t iomaster_config = {
        .eInterfaceMode = AM_HAL_IOM_SPI_MODE,  //! Select the interface mode, SPI or I2C
        .ui32ClockFreq = SPI_CLOCK_FREQ,        //! Select the interface clock frequency
        .eSpiMode  = AM_HAL_IOM_SPI_MODE_3,     //! Select the SPI clock mode (polarity/phase). Ignored for I2C operation.
        .pNBTxnBuf = NULL,
        .ui32NBTxnBufLength = 0,
}
;

    retVal32 = am_hal_iom_initialize(iomaster_instance, &iomaster_handle);
    am_util_stdio_printf("am_hal_iom_initialize returned: %d\n", retVal32);

    retVal32 = am_hal_iom_power_ctrl(iomaster_handle, AM_HAL_SYSCTRL_WAKE, false);
    am_util_stdio_printf("am_hal_iom_power_ctrl returned: %d\n", retVal32);

    retVal32 = am_hal_iom_configure(iomaster_handle, &iomaster_config);
    am_util_stdio_printf("am_hal_iom_configure returned: %d\n", retVal32);

    retVal32 = am_hal_iom_enable(iomaster_handle);
    am_util_stdio_printf("am_hal_iom_enable returned: %d\n", retVal32);


    // Configure pins (make sure they are the right pins for the chosen IOMaster)
    uint8_t pad_MOSI = 0xFF;
    uint8_t pad_MISO = 0xFF;
    uint8_t pad_SCLK = 0xFF;

    uint8_t fncsel_MOSI = 0xFF;
    uint8_t fncsel_MISO = 0xFF;
    uint8_t fncsel_SCLK = 0xFF;

    switch( iomaster_instance ){
    case 0 :
        pad_SCLK = 5; fncsel_SCLK = AM_HAL_PIN_5_M0SCK;
        pad_MOSI = 7; fncsel_MOSI = AM_HAL_PIN_7_M0MOSI;
        pad_MISO = 6; fncsel_MISO = AM_HAL_PIN_6_M0MISO;
        break;

    case 1 :
        pad_SCLK = 8; fncsel_SCLK = AM_HAL_PIN_8_M1SCK;
        pad_MOSI = 10; fncsel_MOSI = AM_HAL_PIN_10_M1MOSI;
        pad_MISO = 9; fncsel_MISO = AM_HAL_PIN_9_M1MISO;
        break;

    case 2 :

        pad_SCLK = 27; fncsel_SCLK = AM_HAL_PIN_27_M2SCK;
        pad_MOSI = 28; fncsel_MOSI = AM_HAL_PIN_28_M2MOSI;
        pad_MISO = 25; fncsel_MISO = AM_HAL_PIN_25_M2MISO;
        break;

    case 3 :
        pad_SCLK = 42; fncsel_SCLK = AM_HAL_PIN_42_M3SCK;
        pad_MOSI = 38; fncsel_MOSI = AM_HAL_PIN_38_M3MOSI;
        pad_MISO = 43; fncsel_MISO = AM_HAL_PIN_43_M3MISO;
        break;

    case 4 :
        pad_SCLK = 39; fncsel_SCLK = AM_HAL_PIN_39_M4SCK;
        pad_MOSI = 44; fncsel_MOSI = AM_HAL_PIN_44_M4MOSI;
        pad_MISO = 40; fncsel_MISO = AM_HAL_PIN_40_M4MISO;
        break;

    case 5 :
        pad_SCLK = 48; fncsel_SCLK = AM_HAL_PIN_48_M5SCK;
        pad_MOSI = 47; fncsel_MOSI = AM_HAL_PIN_47_M5MOSI;
        pad_MISO = 49; fncsel_MISO = AM_HAL_PIN_49_M5MISO;
        break;
    }

    am_hal_gpio_pincfg_t iomaster_pin_config;

    memset((void*)&iomaster_pin_config, 0x00, sizeof(am_hal_gpio_pincfg_t));
    iomaster_pin_config.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA;
    iomaster_pin_config.eGPOutcfg = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
    iomaster_pin_config.uIOMnum = iomaster_instance;

    iomaster_pin_config.uFuncSel = fncsel_SCLK;
    retVal32 = am_hal_gpio_pinconfig(pad_SCLK, iomaster_pin_config);
    am_util_stdio_printf("am_hal_gpio_pinconfig (SCLK) returned: %d\n", retVal32);

    iomaster_pin_config.uFuncSel = fncsel_MOSI;
    retVal32 = am_hal_gpio_pinconfig(pad_MOSI, iomaster_pin_config);
    am_util_stdio_printf("am_hal_gpio_pinconfig (MOSI) returned: %d\n", retVal32);

    iomaster_pin_config.uFuncSel = fncsel_MISO;
    retVal32 = am_hal_gpio_pinconfig(pad_MISO, iomaster_pin_config);
    am_util_stdio_printf("am_hal_gpio_pinconfig (MISO) returned: %d\n", retVal32);



    // Now try to use the SPI port to make sure it is working
    am_hal_iom_transfer_t iomaster_transfer = {0};
    iomaster_transfer.ui32InstrLen = 0;
    iomaster_transfer.ui32Instr = 0;
    iomaster_transfer.bContinue = false;
    iomaster_transfer.ui8RepeatCount = 0;
    iomaster_transfer.ui8Priority = 1;
    iomaster_transfer.ui32PauseCondition = 0;
    iomaster_transfer.ui32StatusSetClr = 0;
#ifdef DIRECTION_TX
    iomaster_transfer.eDirection = AM_HAL_IOM_TX;
    iomaster_transfer.pui32TxBuffer = (uint32_t*)tx_buffer;
    iomaster_transfer.ui32NumBytes = TX_SIZE;
#else
    iomaster_transfer.eDirection = AM_HAL_IOM_RX;
    iomaster_transfer.pui32RxBuffer = (uint32_t*)rx_buffer;
    iomaster_transfer.ui32NumBytes = RX_SIZE;
#endif

    while(1){
        retVal32 = am_hal_iom_blocking_transfer(iomaster_handle, &iomaster_transfer);
        am_util_stdio_printf("am_hal_iom_blocking_transfer returned: %d\n", retVal32);

        am_util_delay_ms(250);
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