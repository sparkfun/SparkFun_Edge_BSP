#include "edge_camera.h"




uint32_t cameraSetup( void ){
    uint32_t retval = 0x00;

    // Setup Pins
    am_hal_gpio_pinconfig(AM_BSP_CAM_XCLK, g_AM_BSP_CAM_XCLK);          // Clock generation for camera

    am_hal_gpio_pinconfig(AM_BSP_CAM_PCLK, g_AM_BSP_CAM_PCLK);          // Interrupts for state
    am_hal_gpio_pinconfig(AM_BSP_CAM_HREF, g_AM_BSP_CAM_HREF);          
    am_hal_gpio_pinconfig(AM_BSP_CAM_VSYNC, g_AM_BSP_CAM_VSYNC);

    am_hal_gpio_pinconfig(AM_BSP_CAM_Y0, g_AM_BSP_CAM_Y0);              // Inputs for data
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y1, g_AM_BSP_CAM_Y1);
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y2, g_AM_BSP_CAM_Y2);
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y3, g_AM_BSP_CAM_Y3);
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y4, g_AM_BSP_CAM_Y4);
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y5, g_AM_BSP_CAM_Y5);
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y6, g_AM_BSP_CAM_Y6);
    am_hal_gpio_pinconfig(AM_BSP_CAM_Y7, g_AM_BSP_CAM_Y7);

    // // Attach interrupts
    // retval = am_hal_gpio_interrupt_register(AM_BSP_CAM_PCLK, &edge_cam_int_pclk);
    // if(retval){ return retval; }
    // retval = am_hal_gpio_interrupt_register(AM_BSP_CAM_HREF, &edge_cam_int_href);
    // if(retval){ return retval; }
    // retval = am_hal_gpio_interrupt_register(AM_BSP_CAM_VSYNC, &edge_cam_int_vsync);
    // if(retval){ return retval; }


    // Enable interrupts
    retval = am_hal_gpio_interrupt_enable( (0b1 << AM_BSP_CAM_PCLK) );
    if(retval){ return retval; }
    retval = am_hal_gpio_interrupt_enable( (0b1 << AM_BSP_CAM_HREF) );
    if(retval){ return retval; }    
    retval = am_hal_gpio_interrupt_enable( (0b1 << AM_BSP_CAM_VSYNC) );
    if(retval){ return retval; }    

    //
    // Enable interrupts.
    //
    NVIC_EnableIRQ(GPIO_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Configure clock source
    //
    am_hal_clkgen_clkout_enable (1, 26); // 26 = HFRC/4, 27=HFRC/8

    return retval;
}


void edge_cam_isr( uint64_t ui64IntMask ){
    // Conditionally call the camera interrupts if their bits are set. Use the priority that makes the most sense

    // if(ui64IntMask & ( 0b1 << (AM_BSP_CAM_PCLK + 4) )){
    if(ui64IntMask & ( 0b1 << (AM_BSP_CAM_PCLK + 0) )){
        edge_cam_int_pclk();
    }
    // if(ui64IntMask & ( 0b1 << (AM_BSP_CAM_HREF + 4) )){
    if(ui64IntMask & ( 0b1 << (AM_BSP_CAM_HREF + 0) )){
        edge_cam_int_href();
    }
    // if(ui64IntMask & ( 0b1 << (AM_BSP_CAM_VSYNC + 4) )){
    if(ui64IntMask & ( 0b1 << (AM_BSP_CAM_VSYNC + 0) )){
        edge_cam_int_vsync();
    }
}

void edge_cam_int_pclk( void ){
    am_bsp_uart_string_print("PCLK INT!\r\n");
}

void edge_cam_int_href( void ){
    am_bsp_uart_string_print("HREF INT!\r\n");
}

void edge_cam_int_vsync( void ){
    am_bsp_uart_string_print("VSYNC INT!\r\n");
}





// Boneyard. Code that I hate to see go to waste

// Trying to set up the clock source
//     // AM_HAL_CLKGEN_CONTROL_XTAL_START
// am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_START, 0);

// // AM_HAL_CLKGEN_CLKOUT_XTAL_8192
// // AM_HAL_CLKGEN_CLKOUT_XTAL_1024
//     am_hal_clkgen_clkout_enable(true, 0x17); // Should go out onto the pin that we've configured for this purpose

//     am_hal_clkgen_status_t rxClkStat;
//     am_hal_clkgen_status_get(&rxClkStat);

//     am_util_stdio_printf("Clkgen status: SysclkFreq - %d, eRTCOSC - %d, xtalfail %d\r\n", rxClkStat.ui32SysclkFreq, rxClkStat.eRTCOSC, rxClkStat.bXtalFailure);

    // memset(clkbuff, 0xAA, CLKBUFNUMBYTES);

    // am_hal_iom_config_t spiConfig =
    // {
    //     .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
    //     .eSpiMode = AM_HAL_IOM_SPI_MODE_3,
    //     .ui32ClockFreq = AM_HAL_IOM_16MHZ,
    //     .pNBTxnBuf = clkbuff,
    //     .ui32NBTxnBufLength = CLKBUFNUMBYTES,
    // };

    // //
    // // Configure the IOM pins.
    // //
    // am_hal_gpio_pinconfig(AM_BSP_CAM_XCLK,  g_AM_BSP_CAM_XCLK); // 7

    // // Initialize the IOM.
    // retval = am_hal_iom_initialize(AM_BSP_CAM_CLKGEN_IOM, &clkbufiomHandle);
    // if (retval != AM_HAL_STATUS_SUCCESS) return retval; // -1;

    // retval = am_hal_iom_power_ctrl(clkbufiomHandle, AM_HAL_SYSCTRL_WAKE, false);
    // if (retval != AM_HAL_STATUS_SUCCESS) return retval; // -2;
    
    // // Set the required configuration settings for the IOM.
    // //
    // retval = am_hal_iom_configure(clkbufiomHandle, &spiConfig);
    // if (retval != AM_HAL_STATUS_SUCCESS) return retval; // -3;


    // // Enable interrupts for DMA complete
    // retval = am_hal_iom_interrupt_enable(clkbufiomHandle, AM_HAL_IOM_INT_DCMP);
    // if (retval != AM_HAL_STATUS_SUCCESS) return retval;

    // //
    // // Enable the IOM.
    // //
    // retval = am_hal_iom_enable(clkbufiomHandle);
    // if (retval != AM_HAL_STATUS_SUCCESS) return retval; // -4;

    // // Enable interrupts for IOM0
    // NVIC_EnableIRQ(IOMSTR0_IRQn);

    // // // Start clocking out
    // // edge_cam_start_clkbuf_xfer();


    // init_timerB4_for_camera();


    // //
    // // Configure the output pin.
    // //
    // am_hal_ctimer_output_config(AM_BSP_CAM_XCLK_TIMER,
    //                             AM_BSP_CAM_XCLK_TIMER_SEG,
    //                             AM_BSP_CAM_XCLK,
    //                             AM_HAL_CTIMER_OUTPUT_NORMAL,
    //                             AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA);

    // //
    // // Configure a timer to drive XCLK.
    // //
    // am_hal_ctimer_config_single(AM_BSP_CAM_XCLK_TIMER,                                      // ui32TimerNumber
    //                             AM_BSP_CAM_XCLK_TIMER_SEG,                                  // ui32TimerSegment
    //                             ( AM_HAL_CTIMER_XT_DIV4 /*AM_HAL_CTIMER_HFRC_12MHZ*/ | AM_HAL_CTIMER_FN_REPEAT ) );    // ui32ConfigVal

    // am_hal_ctimer_period_set(AM_BSP_CAM_XCLK_TIMER,
    //                          AM_BSP_CAM_XCLK_TIMER_SEG, 1, 0);

    // am_hal_ctimer_start(AM_BSP_CAM_XCLK_TIMER, AM_BSP_CAM_XCLK_TIMER_SEG);

    // am_hal_clkgen_clkout_enable(1, 26); // 26 = HFRC/4, 27=HFRC/8
    // am_hal_clkgen_clkout_enable(true, 0x17); // Should go out onto the pin that we've configured for this purpose




    // // We will use a DMA buffer to send out the clock data 
// #define CLKBUFFLEN_4 256
// uint32_t clkbuff[CLKBUFFLEN_4];
// void * clkbufiomHandle;
// #define CLKBUFNUMBYTES (sizeof(uint32_t)*CLKBUFFLEN_4)

// am_hal_iom_transfer_t clkbufTransferDefault = 
// {
//     .ui32InstrLen = 0, // 0 -- 8-bit transfers
//     .ui32Instr = 0, // offset
//     .ui32NumBytes = CLKBUFNUMBYTES,
//     .eDirection = AM_HAL_IOM_TX,
//     .pui32TxBuffer = (uint32_t*)&clkbuff[0],
//     .pui32RxBuffer = NULL,
//     .bContinue = false,
//     .ui8RepeatCount = 0,
//     .ui8Priority = 1,
//     .ui32PauseCondition = 0,
//     .ui32StatusSetClr = 0
// };

// void clkbuf_xfer_callback(void *pCallbackCtxt, uint32_t transactionStatus){
//     // am_util_stdio_printf("clkbuf xfer callback!\n");
//     // // edge_cam_start_clkbuf_xfer(); // Restart the transfer
// }

// void edge_cam_start_clkbuf_xfer( void ){
//     am_hal_iom_nonblocking_transfer(clkbufiomHandle, &clkbufTransferDefault, clkbuf_xfer_callback, NULL);
// }

// uint32_t temp_test( void ){

//     am_hal_iom_transfer_t iomTransfer = clkbufTransferDefault;
//     // iomTransfer.uPeerInfo.ui32I2CDevAddr = AM_BSP_I2C_ACCELEROMETER_ADDRESS; 
//     iomTransfer.ui32InstrLen = 0;           // Number of 8-bit transfers for the instruction phase
//     iomTransfer.ui32Instr = 0;    // Sub-address is the one 8-bit instruction
//     iomTransfer.ui32NumBytes = CLKBUFNUMBYTES;         // Transfer len bytes of data
//     iomTransfer.eDirection = AM_HAL_IOM_TX;
//     iomTransfer.pui32TxBuffer = (uint32_t*)&clkbuff[0];
//     iomTransfer.pui32RxBuffer = NULL;
//     iomTransfer.bContinue = false;          // Do release the bus after this transfer
//     iomTransfer.ui8RepeatCount = 0;         // ?
//     iomTransfer.ui8Priority = 1;            // ?
//     iomTransfer.ui32PauseCondition = 0;     // ?
//     iomTransfer.ui32StatusSetClr = 0;       // ?


//     // return am_hal_iom_nonblocking_transfer(clkbufiomHandle, &clkbufTransferDefault, clkbuf_xfer_callback, NULL);
//     return am_hal_iom_blocking_transfer(clkbufiomHandle, &iomTransfer);
// }


// void init_timerB4_for_camera( void )
// {
//     //
//     // Start a timer to trigger the ADC periodically (1 second).
//     //
//     am_hal_ctimer_config_single(4, AM_HAL_CTIMER_TIMERB,
//                                 AM_HAL_CTIMER_HFRC_12MHZ    |
//                                 AM_HAL_CTIMER_FN_REPEAT     |
//                                 /*AM_HAL_CTIMER_INT_ENABLE*/);

//     // am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERB4);

//     am_hal_ctimer_period_set(4, AM_HAL_CTIMER_TIMERB, 2, 1);

//     am_hal_ctimer_output_config(uint32_t ui32TimerNumber,
//                                             uint32_t ui32TimerSegment,
//                                             uint32_t ui32PadNum,
//                                             uint32_t eOutputType,
//                                             uint32_t eDriveStrength);

//     //
//     // Start the timer.
//     //
//     am_hal_ctimer_start(4, AM_HAL_CTIMER_TIMERB);
// }