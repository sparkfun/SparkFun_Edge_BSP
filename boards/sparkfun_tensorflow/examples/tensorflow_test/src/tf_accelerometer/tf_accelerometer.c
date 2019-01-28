#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "tf_accelerometer.h"

int initAccelerometer(void)
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