#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "tf_accelerometer.h"

// #include <string.h> // Temporary, just for memset

#define TF_ACC_MAX_XFER_SIZE 128
uint32_t i2cTX[TF_ACC_MAX_XFER_SIZE]; // = {0, 0}; // WHO_AM_I register
uint32_t i2cRX[TF_ACC_MAX_XFER_SIZE]; // = {0};

void * iomHandle;

lis2dh12_ctx_t dev_ctx;

am_hal_iom_transfer_t iomTransferDefault = 
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


static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_init(void);



// int initAccelerometer(void)
// {
//     void * iomHandle;
//     uint32_t iom = AM_BSP_I2C_ACCELEROMETER_IOM; // IO module 4 -- switch to 3 if using IOM3
//     uint32_t retVal32 = 0;
//     uint32_t i2cTX[2] = {0, 0}; // WHO_AM_I register
//     uint32_t i2cRX[1] = {0};

//     am_hal_iom_config_t i2cConfig =
//     {
//         .eInterfaceMode = AM_HAL_IOM_I2C_MODE,
//         .ui32ClockFreq = AM_HAL_IOM_100KHZ
//     };

//     // Initialize the IOM.
//     retVal32 = am_hal_iom_initialize(iom, &iomHandle);
//     if (retVal32 != AM_HAL_STATUS_SUCCESS) return -1;

//     retVal32 = am_hal_iom_power_ctrl(iomHandle, AM_HAL_SYSCTRL_WAKE, false);
//     if (retVal32 != AM_HAL_STATUS_SUCCESS) return -2;
    
//     // Set the required configuration settings for the IOM.
//     //
//     retVal32 = am_hal_iom_configure(iomHandle, &i2cConfig);
//     if (retVal32 != AM_HAL_STATUS_SUCCESS) return -3;

//     //
//     // Configure the IOM pins.
//     //
//     am_hal_gpio_pinconfig(AM_BSP_I2C_ACCELEROMETER_SDA_PIN,  g_AM_BSP_GPIO_IOM4_SCL); // 39
//     am_hal_gpio_pinconfig(AM_BSP_I2C_ACCELEROMETER_SCL_PIN,  g_AM_BSP_GPIO_IOM4_SDA); // 40

//     //
//     // Enable the IOM.
//     //
//     retVal32 = am_hal_iom_enable(iomHandle);
//     if (retVal32 != AM_HAL_STATUS_SUCCESS) return -4;


//     am_hal_iom_transfer_t iomTransfer = 
//     {
//         .uPeerInfo.ui32I2CDevAddr = AM_BSP_I2C_ACCELEROMETER_ADDRESS,
//         .ui32InstrLen = 0, // 0 -- 8-bit transfers
//         .ui32Instr = 0, // offset
//         .ui32NumBytes = 1,
//         .eDirection = AM_HAL_IOM_TX,
//         .pui32TxBuffer = i2cTX,
//         .pui32RxBuffer = i2cRX,
//         .bContinue = true,
//         .ui8RepeatCount = 0,
//         .ui8Priority = 1,
//         .ui32PauseCondition = 0,
//         .ui32StatusSetClr = 0
//     };

//     i2cTX[0] = 0x0F; // WHO_AM_I register
//     retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
//     if (retVal32 != 0)
//     {
//         am_util_stdio_printf("Send err: %d\n", retVal32);
//         return retVal32;
//     }

//     iomTransfer.eDirection = AM_HAL_IOM_RX;
//     iomTransfer.bContinue = false;
//     iomTransfer.ui32NumBytes = 1;
    
//     retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
//     if (retVal32 != 0)
//     {
//         am_util_stdio_printf("Receive err: %d\n", retVal32);
//         return retVal32;
//     }

//     am_util_stdio_printf("Accelerometer WHO_AM_I (should be 0x33): 0x%x\n", i2cRX[0]);

//     // Power-down the accelerometer
//     i2cTX[0] = 0x20; // Register 0x20 (CTRL_REG1)
//     i2cTX[1] = 0x00; // Write value 0 (power-down mode)
//     iomTransfer.eDirection = AM_HAL_IOM_TX;
//     iomTransfer.bContinue = false;
//     iomTransfer.ui32NumBytes = 2;
    
//     retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);

//     return 0;
// }


int initAccelerometer(void)
{
    // static axis3bit16_t data_raw_acceleration;
    // static axis1bit16_t data_raw_temperature;
    // static float acceleration_mg[3];
    // static float temperature_degC;
    static uint8_t whoamI;

    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    // dev_ctx.handle = NULL; // = &hi2c1;  // No need for this

    platform_init();

    lis2dh12_device_id_get(&dev_ctx, &whoamI);
    if (whoamI != LIS2DH12_ID)
    {
        while(1)
        {
        /* manage here device not found */
        }
    }
    am_util_stdio_printf("Whoami (should be 0x33): 0x%2x\n", whoamI);

    /*
    *  Enable Block Data Update
    */
    lis2dh12_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);


    /*
    *   Enable temperature sensor
    */
    lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);


    /*
    * Set Output Data Rate to 25Hz
    */
    lis2dh12_data_rate_set(&dev_ctx, LIS2DH12_ODR_25Hz);

    /*
    * Set full scale to 2g
    */  
    lis2dh12_full_scale_set(&dev_ctx, LIS2DH12_2g);

    /*
    * Enable temperature sensor
    */   
    lis2dh12_temperature_meas_set(&dev_ctx, LIS2DH12_TEMP_ENABLE);

    /*
    * Set device in continuous mode with 12 bit resol.
    */   
    lis2dh12_operating_mode_set(&dev_ctx, LIS2DH12_HR_12bit);


    // /*
    // * Read samples in polling mode (no int)
    // */
    // while(1)
    // {
    //     lis2dh12_reg_t reg;

    //     /*
    //     * Read output only if new value available
    //     */
    //     lis2dh12_xl_data_ready_get(&dev_ctx, &reg.byte);
    //     if (reg.byte)
    //     {
    //         /* Read accelerometer data */
    //         memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
    //         lis2dh12_acceleration_raw_get(&dev_ctx, data_raw_acceleration.u8bit);
    //         acceleration_mg[0] =
    //         lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[0]);
    //         acceleration_mg[1] =
    //         lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[1]);
    //         acceleration_mg[2] =
    //         lis2dh12_from_fs2_hr_to_mg(data_raw_acceleration.i16bit[2]);
            
    //         am_util_stdio_printf("Acceleration [mg]:%04.2f, %04.2f, %04.2f\r\n",
    //                 acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
    //     }

    //     // lis2dh12_temp_data_ready_get(&dev_ctx, &reg.byte);      
    //     // if (reg.byte)    
    //     // {
    //     //     /* Read temperature data */
    //     //     memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
    //     //     lis2dh12_temperature_raw_get(&dev_ctx, data_raw_temperature.u8bit);
    //     //     temperature_degC =
    //     //     lis2dh12_from_lsb_hr_to_celsius(data_raw_temperature.i16bit);
            
    //     //     am_util_stdio_printf("Temperature [degC]:%6.2f\r\n", temperature_degC);
    //     // }
    // }



    // // Power-down the accelerometer
    // i2cTX[0] = 0x20; // Register 0x20 (CTRL_REG1)
    // i2cTX[1] = 0x00; // Write value 0 (power-down mode)
    // iomTransfer.eDirection = AM_HAL_IOM_TX;
    // iomTransfer.bContinue = false;
    // iomTransfer.ui32NumBytes = 2;
    
    // retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);

    return 0;
}




/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
    if(len > TF_ACC_MAX_XFER_SIZE){ return 1; } // Error, length too great

    uint32_t retVal32 = 0;

    am_hal_iom_transfer_t iomTransfer = iomTransferDefault;
    // iomTransfer.uPeerInfo.ui32I2CDevAddr = AM_BSP_I2C_ACCELEROMETER_ADDRESS; 
    iomTransfer.ui32InstrLen = 1;           // Number of 8-bit transfers for the instruction phase
    iomTransfer.ui32Instr = (reg |0x80);    // Sub-address is the one 8-bit instruction
    iomTransfer.ui32NumBytes = len;         // Transfer len bytes of data
    iomTransfer.eDirection = AM_HAL_IOM_TX;
    iomTransfer.pui32TxBuffer = i2cTX;
    iomTransfer.pui32RxBuffer = i2cRX;
    iomTransfer.bContinue = false;          // Do release the bus after this transfer
    iomTransfer.ui8RepeatCount = 0;         // ?
    iomTransfer.ui8Priority = 1;            // ?
    iomTransfer.ui32PauseCondition = 0;     // ?
    iomTransfer.ui32StatusSetClr = 0;       // ?

    // Copy the send data to the tx buffer
    if(bufp == NULL){
        am_util_stdio_printf("No TX buffer provided\n");
        return 1; // Error no data to transmit
    }
    while(len--){
        *(i2cTX + len) = *(bufp + len);
    }
    
    retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
    if (retVal32 != 0)
    {
        am_util_stdio_printf("Send err (phase 1): %d\n", retVal32);
        return retVal32;
    }
    
    return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
    if(len > TF_ACC_MAX_XFER_SIZE){ return 1; } // Error, length too great
    if(bufp == NULL){ am_util_stdio_printf("No RX buffer provided\n"); return 1; } // Error, no rx buffer provided

    uint32_t retVal32 = 0;

    am_hal_iom_transfer_t iomTransfer = iomTransferDefault;
    // iomTransfer.uPeerInfo.ui32I2CDevAddr = AM_BSP_I2C_ACCELEROMETER_ADDRESS; 
    iomTransfer.ui32InstrLen = 1;           // 8-bit transfers
    iomTransfer.ui32Instr = (reg | 0x80);   // Offset;
    iomTransfer.ui32NumBytes = 0;           // No data, just the instruction byte which is the sub-address
    iomTransfer.eDirection = AM_HAL_IOM_TX;
    iomTransfer.pui32TxBuffer = NULL;
    iomTransfer.pui32RxBuffer = NULL;
    iomTransfer.bContinue = true;           // Don't release the I2C bus after this one...
    iomTransfer.ui8RepeatCount = 0;         // ?
    iomTransfer.ui8Priority = 1;            // ?
    iomTransfer.ui32PauseCondition = 0;     // ?
    iomTransfer.ui32StatusSetClr = 0;       // ?

    // Send the first one...
    retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
    if (retVal32 != 0)
    {
        am_util_stdio_printf("Send err: %d\n", retVal32);
        return retVal32;
    }

    // Change direction, and add the rx buffer
    iomTransfer.eDirection = AM_HAL_IOM_RX;
    iomTransfer.pui32RxBuffer = bufp;       // Link in the RX buffer
    iomTransfer.ui32NumBytes = len;         // How many bytes to receive
    iomTransfer.bContinue = false;
    
    retVal32 = am_hal_iom_blocking_transfer(iomHandle, &iomTransfer);
    if (retVal32 != 0)
    {
        am_util_stdio_printf("Receive err: %d\n", retVal32);
        return retVal32;
    }
    
    return 0;
}

/*
 * @brief  platform specific initialization (platform dependent)
 */
static void platform_init(void)
{
    uint32_t retVal32 = 0;

    am_hal_iom_config_t i2cConfig =
    {
        .eInterfaceMode = AM_HAL_IOM_I2C_MODE,
        .ui32ClockFreq = AM_HAL_IOM_100KHZ
    };

    // Initialize the IOM.
    retVal32 = am_hal_iom_initialize(AM_BSP_I2C_ACCELEROMETER_IOM, &iomHandle);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return; // -1;

    retVal32 = am_hal_iom_power_ctrl(iomHandle, AM_HAL_SYSCTRL_WAKE, false);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return; // -2;
    
    // Set the required configuration settings for the IOM.
    //
    retVal32 = am_hal_iom_configure(iomHandle, &i2cConfig);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return; // -3;

    //
    // Configure the IOM pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_I2C_ACCELEROMETER_SDA_PIN,  g_AM_BSP_GPIO_IOM4_SCL); // 39
    am_hal_gpio_pinconfig(AM_BSP_I2C_ACCELEROMETER_SCL_PIN,  g_AM_BSP_GPIO_IOM4_SDA); // 40

    //
    // Enable the IOM.
    //
    retVal32 = am_hal_iom_enable(iomHandle);
    if (retVal32 != AM_HAL_STATUS_SUCCESS) return; // -4;
}
