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

static int boardSetup(void);
static int testAccelerometer(void);
static int testADC(void);

//
// ADC Device Handle.
//
static void *g_ADCHandle;

const am_hal_gpio_pincfg_t g_AM_PIN_29_ADCSE1 =
{
    .uFuncSel       = AM_HAL_PIN_29_ADCSE1,
};

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    boardSetup();

    am_util_stdio_terminal_clear();

    am_util_stdio_printf("SparkFun Tensorflow Debug Output (SWO)\r\n");
    am_bsp_uart_string_print("Hello, UART!\r\n");

    testAccelerometer();

    testADC();

    while (1) 
    {

    }

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
    am_bsp_debug_printf_disable();
}

static int boardSetup(void)
{
    //
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();


    //
    // Initialize the printf interface for ITM/SWO output.
    //
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

static int testAccelerometer(void)
{
    void * iomHandle;
    uint32_t iom = AM_BSP_I2C_ACCELEROMETER_IOM; // IO module 4 -- switch to 3 if using IOM3
    uint32_t retVal32 = 0;
    uint32_t i2cTX[2] = {0, 0}; // WHO_AM_I register
    uint32_t i2cRX[1] = {0};

    am_hal_iom_config_t i2cConfig =
    {
        .eInterfaceMode = AM_HAL_IOM_I2C_MODE,
        .ui32ClockFreq = AM_HAL_IOM_100KHZ
    };

    // Initialize the IOM.
    retVal32 = am_hal_iom_initialize(iom, &iomHandle);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return -1;

    retVal32 = am_hal_iom_power_ctrl(iomHandle, AM_HAL_SYSCTRL_WAKE, false);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return -2;
    
    // Set the required configuration settings for the IOM.
    //
    retVal32 = am_hal_iom_configure(iomHandle, &i2cConfig);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return -3;

    //
    // Configure the IOM pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_I2C_ACCELEROMETER_SDA_PIN,  g_AM_BSP_GPIO_IOM4_SCL); // 39
    am_hal_gpio_pinconfig(AM_BSP_I2C_ACCELEROMETER_SCL_PIN,  g_AM_BSP_GPIO_IOM4_SDA); // 40

    //
    // Enable the IOM.
    //
    retVal32 = am_hal_iom_enable(iomHandle);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return -4;


    am_hal_iom_transfer_t iomTransfer = 
    {
        .uPeerInfo.ui32I2CDevAddr = AM_BSP_I2C_ACCELEROMETER_ADDRESS,
        .ui32InstrLen = 0, // 0 -- 8-bit transfers
        .ui32Instr = 0, // offset
        .ui32NumBytes = 1,
        .eDirection = AM_HAL_IOM_TX,
        .pui32TxBuffer = i2cTX,
        .pui32RxBuffer = i2cRX,
        .bContinue = true,
        .ui8RepeatCount = 0,
        .ui8Priority = 1,
        .ui32PauseCondition = 0,
        .ui32StatusSetClr = 0
    };

    i2cTX[0] = 0x0F; // WHO_AM_I register
    retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
    if (retVal32 != 0)
    {
        am_util_stdio_printf("Send err: %d\n", retVal32);
        return retVal32;
    }

    iomTransfer.eDirection = AM_HAL_IOM_RX;
    iomTransfer.bContinue = false;
    iomTransfer.ui32NumBytes = 1;
    
    retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
    if (retVal32 != 0)
    {
        am_util_stdio_printf("Receive err: %d\n", retVal32);
        return retVal32;
    }

    am_util_stdio_printf("Accelerometer WHO_AM_I (should be 0x33): 0x%x\n", i2cRX[0]);

    // Power-down the accelerometer
    i2cTX[0] = 0x20; // Register 0x20 (CTRL_REG1)
    i2cTX[1] = 0x00; // Write value 0 (power-down mode)
    iomTransfer.eDirection = AM_HAL_IOM_TX;
    iomTransfer.bContinue = false;
    iomTransfer.ui32NumBytes = 2;
    
    retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);

    return 0;
}

static int testADC(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    am_hal_gpio_pinconfig(29, g_AM_PIN_29_ADCSE1);

    //
    // Initialize the ADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the ADC instance failed.\n");
    }

    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                            AM_HAL_SYSCTRL_WAKE,
                                                            false) )
    {
        am_util_stdio_printf("Error - ADC power on failed.\n");
    }

    //
    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC;
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eReference         = AM_HAL_ADC_REFSEL_INT_2P0;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_POWER;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
    {
        am_util_stdio_printf("Error - configuring ADC failed.\n");
    }

    //
    // Set up an ADC slot
    //
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE1;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 0 failed.\n");
    }

    //
    // Enable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC failed.\n");
    }

    uint32_t    ui32NumSamples = 100;
    am_hal_adc_sample_t Sample[100];
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle,
                                                         NULL,
                                                         &ui32NumSamples,
                                                         Sample))
    
    am_util_stdio_printf("ADC Slot =  %d\n", Sample[0].ui32Slot);
    //am_util_stdio_printf("ADC Value = %d\n", Sample[0].ui32Sample);
    for (int i = 0; i < 100; i++) 
    {
        am_util_stdio_printf("%8.8X ", Sample[i].ui32Slot);
    }
        am_util_stdio_printf("\n");

    return 0;
}