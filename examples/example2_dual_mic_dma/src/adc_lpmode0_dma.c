//*****************************************************************************
//
//! @file adc_lpmode0_dma.c
//!
//! @brief Example that takes samples with the ADC at high-speed.
//!
//! This example shows the CTIMER-A3 triggering repeated 16 KHz samples
//! based on an external input at 12MHz in LPMODE0.  The example uses the
//! CTIMER-A3 to trigger ADC sampling.  Each data point is transferred
//! from the ADC FIFO into an SRAM buffer using DMA. In the code you will
//! also find hints on how to enable 128 sample average and other options.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2019, Ambiq Micro
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
// 
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision v2.0.0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"


// Edge Board Pin Definitions
#define SF_EDGE_PIN_MIC0    11
const am_hal_gpio_pincfg_t g_SF_EDGE_PIN_MIC0 =
{
    .uFuncSel       = AM_HAL_PIN_11_ADCSE2,
};
#define SF_EDGE_PIN_MIC1    29
const am_hal_gpio_pincfg_t g_SF_EDGE_PIN_MIC1 =
{
    .uFuncSel       = AM_HAL_PIN_29_ADCSE1,
};





//*****************************************************************************
//
// Define a circular buffer to hold the ADC samples
//
//*****************************************************************************
#define ADC_EXAMPLE_DEBUG   1

//
// ADC Sample buffers.
//

#define NUM_SLOTS (2)
#define ADC_SAMPLES_PER_SLOT (1024)                                 

#define ADC_SAMPLE_BUF_SIZE (NUM_SLOTS * ADC_SAMPLES_PER_SLOT)      // Note: there seem to be memry limitations after 2048 channel-samples. Only a portion of RAM gets turned on though, it seems.


uint32_t g_ui32ADCSampleBuffer[ADC_SAMPLE_BUF_SIZE];                // This is the raw buffer that is filled by the ADC during DMA  

am_hal_adc_sample_t SampleBuffer0[ADC_SAMPLES_PER_SLOT];            // These buffers are basically the same exact thing expect the type
am_hal_adc_sample_t SampleBuffer1[ADC_SAMPLES_PER_SLOT];            // is a structure with names for the actual value (.ui32Sample) and the slot number (.ui32Slot)
                                                                    // And actually I just noticed that that type has TWO 32-bit fields in each element

//
// ADC Device Handle.
//
static void *g_ADCHandle;
// static void *g_ADCHandle1;

//
// ADC DMA complete flag.
//
volatile bool                   g_bADCDMAComplete;

//
// ADC DMA error flag.
//
volatile bool                   g_bADCDMAError;


//*****************************************************************************
//
// Interrupt handler for the ADC.
//
//*****************************************************************************
void
am_adc_isr(void)
{
    uint32_t ui32IntMask;

    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading ADC0 interrupt status\n");
    }

    //
    // Clear the ADC interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing ADC0 interrupt status\n");
    }

    //
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_ADC_INT_DCMP)
    {
        g_bADCDMAComplete = true;
    }

    //
    // If we got a DMA error, set the flag.
    //
    if (ui32IntMask & AM_HAL_ADC_INT_DERR)
    {
        g_bADCDMAError = true;
    }
}

//*****************************************************************************
//
// Set up the core for sleeping, and then go to sleep.
//
//*****************************************************************************
void
sleep(void)
{
    //
    // Disable things that can't run in sleep mode.
    //
#if (0 == ADC_EXAMPLE_DEBUG)
    am_bsp_debug_printf_disable();
#endif

    //
    // Go to Deep Sleep.
    //
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    //
    // Re-enable peripherals for run mode.
    //
#if (0 == ADC_EXAMPLE_DEBUG)
    am_bsp_debug_printf_enable();
#endif
}

//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config_dma(void)
{
    am_hal_adc_dma_config_t       ADCDMAConfig;

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    ADCDMAConfig.bDynamicPriority = true;
    ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
    ADCDMAConfig.bDMAEnable = true;
    ADCDMAConfig.ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
    ADCDMAConfig.ui32TargetAddress = (uint32_t)g_ui32ADCSampleBuffer;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
    }

    //
    // Reset the ADC DMA flags.
    //
    g_bADCDMAComplete = false;
    g_bADCDMAError = false;
}

//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config0(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    //
    // Initialize the ADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the ADC0 instance failed.\n");
    }

    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - ADC0 power on failed.\n");
    }

    //
    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC_DIV2; // AM_HAL_ADC_CLKSEL_HFRC;
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eReference         = AM_HAL_ADC_REFSEL_INT_2P0;  //AM_HAL_ADC_REFSEL_INT_1P5;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
    {
        am_util_stdio_printf("Error - configuring ADC0 failed.\n");
    }


    //
    // Set up an ADC slot (2)
    //
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;  //AM_HAL_ADC_SLOT_AVG_128;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
    // ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE2;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 2, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 2 failed.\n");
    }

    //
    // Set up an ADC slot (1)
    //
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;  //AM_HAL_ADC_SLOT_AVG_128;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
    // ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE1;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 1, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 1 failed.\n");
    }

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    adc_config_dma();

    //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP );

    //
    // Enable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC0 failed.\n");
    }
}


//*****************************************************************************
//
// Initialize the ADC repetitive sample timer A3.
//
//*****************************************************************************
void
init_timerA3_for_ADC(void)
{
    //
    // Start a timer to trigger the ADC periodically (1 second).
    //
    am_hal_ctimer_config_single(3, AM_HAL_CTIMER_TIMERA,
                                AM_HAL_CTIMER_HFRC_12MHZ    |
                                AM_HAL_CTIMER_FN_REPEAT     |
                                AM_HAL_CTIMER_INT_ENABLE);

    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA3);

    am_hal_ctimer_period_set(3, AM_HAL_CTIMER_TIMERA, 750, 374);

    //
    // Enable the timer A3 to trigger the ADC directly
    //
    am_hal_ctimer_adc_trigger_enable();

    //
    // Start the timer.
    //
    am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);
}


//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    am_bsp_uart_printf_enable();                                            // Enable UART - will set debug output to UART. Replaces icm print enable
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("SparkFun Edge DMA ADC Test\n");
    am_util_stdio_printf("Compiled on %s, %s\n\n", __DATE__, __TIME__);



    //
    // Set the clock frequency.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0))
    {
        am_util_stdio_printf("Error - configuring the system clock failed.\n");
    }


    //
    // Set the default cache configuration and enable it.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_cachectrl_config(&am_hal_cachectrl_defaults))
    {
        am_util_stdio_printf("Error - configuring the system cache failed.\n");
    }
    if (AM_HAL_STATUS_SUCCESS != am_hal_cachectrl_enable())
    {
        am_util_stdio_printf("Error - enabling the system cache failed.\n");
    }

    // //
    // // Configure the board for low power operation.
    // //
    // am_bsp_low_power_init();

    // I commented out the low_power_init function because it had cancelled UART output. 
    // I'm wondering if this might indicate that the wrong BSP is being used? Because boardSetup() from edge_test example also calls 'bsp_low_power_init()' but the UART stays alive


    //
    // Enable only the first 512KB bank of Flash (0).  Disable Flash(1)
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_512K))
    {
        am_util_stdio_printf("Error - configuring the flash memory failed.\n");
    }

    //
    // Enable the first 32K of TCM SRAM.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_SRAM_32K_DTCM))
    {
        am_util_stdio_printf("Error - configuring the SRAM failed.\n");
    }

    am_util_stdio_printf("Ayyy boo\n");


    //
    // Start the CTIMER A3 for timer-based ADC measurements.
    //
    init_timerA3_for_ADC();

    //
    // Enable interrupts.
    //
    NVIC_EnableIRQ(ADC_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Set a pin(s) to act as our ADC input
    //
    am_hal_gpio_pinconfig(SF_EDGE_PIN_MIC0, g_SF_EDGE_PIN_MIC0);
    am_hal_gpio_pinconfig(SF_EDGE_PIN_MIC1, g_SF_EDGE_PIN_MIC1);

    //
    // Configure the ADC
    //
    adc_config0();

    //
    // Trigger the ADC sampling for the first time manually.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
    {
        am_util_stdio_printf("Error - triggering the ADC0 failed.\n");
    }

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("ADC Example with 1.2Msps and LPMODE=0\n");

    //
    // Allow time for all printing to finish.
    //
    am_util_delay_ms(10);

    //
    // We are done printing. Disable debug printf messages on ITM.
    //
#if (0 == ADC_EXAMPLE_DEBUG)
    am_bsp_debug_printf_disable();
#endif

    am_util_stdio_printf("Entering the loop...\n");

    //
    // Loop forever.
    //
    while(1)
    {
 
        //
        // Go to Deep Sleep.
        //
        if (!g_bADCDMAComplete)
        {
            sleep();
        }

        //
        // Check for DMA errors.
        //
        if (g_bADCDMAError)
        {
            am_util_stdio_printf("DMA Error occured\n");
            while(1);
        }

        //
        // Check if the ADC DMA completion interrupt occurred.
        //
        if (g_bADCDMAComplete)
        {
#if ADC_EXAMPLE_DEBUG
            {
                // uint32_t        ui32SampleCount;
                am_util_stdio_printf("DMA Complete\n");
                // ui32SampleCount = ADC_SAMPLE_BUF_SIZE;


                // if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle,
                //                                                      g_ui32ADCSampleBuffer,
                //                                                      &ui32SampleCount,
                //                                                      SampleBuffer0))
                // {
                //     am_util_stdio_printf("Error - failed to process samples _0_.\n");
                // }

                // Note: the hal_adc_samples_read function is supposed to accept an argument to specify which slot to read the samples from. 
                //       But it doesn't seem to actually support this feature.... I think we could figure it out on our own as long as the 
                //      .ui32Slot field actually gets set

                // I tried to write a function to take the raw samples fromt the ADC and sort them into the right buffer. It was close but I bailed to just hard-code it for now

                // For slot 1:
                uint32_t slotCount = 0;
                for( uint32_t indi = 0; indi < ADC_SAMPLE_BUF_SIZE; indi++){
                    am_hal_adc_sample_t temp;

                    temp.ui32Slot   = AM_HAL_ADC_FIFO_SLOT(g_ui32ADCSampleBuffer[indi]);
                    temp.ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(g_ui32ADCSampleBuffer[indi]);

                    if( temp.ui32Slot == 1 ){
                        SampleBuffer1[slotCount] = temp;
                        slotCount++;
                    }
                }


                slotCount = 0;
                for( uint32_t indi = 0; indi < ADC_SAMPLE_BUF_SIZE; indi++){
                    am_hal_adc_sample_t temp;

                    temp.ui32Slot   = AM_HAL_ADC_FIFO_SLOT(g_ui32ADCSampleBuffer[indi]);
                    temp.ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(g_ui32ADCSampleBuffer[indi]);

                    if( temp.ui32Slot == 2 ){
                        SampleBuffer0[slotCount] = temp;
                        slotCount++;
                    }
                }

                // Print out the results over UART for visual verification
                for(uint32_t indi = 0; indi < ADC_SAMPLES_PER_SLOT; indi++){
                    // am_util_stdio_printf("%d, %d\n", SampleBuffer0[indi].ui32Slot, SampleBuffer1[indi].ui32Slot);           // Show slot numbers

                    am_util_stdio_printf("%d, %d\n", SampleBuffer0[indi].ui32Sample, SampleBuffer1[indi].ui32Sample);    // Show values
                }

            }
#endif

            //
            // Reset the DMA completion and error flags.
            //
            g_bADCDMAComplete = false;

            //
            // Re-configure the ADC DMA.
            //
            adc_config_dma();

            //
            // Clear the ADC interrupts.
            //
            if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, 0xFFFFFFFF))
            {
                am_util_stdio_printf("Error - clearing the ADC0 interrupts failed.\n");
            }

            //
            // Trigger the ADC sampling for the first time manually.
            //
            if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
            {
                am_util_stdio_printf("Error - triggering the ADC0 failed.\n");
            }

        } // if ()
    } // while()
}










// fin










// An incomplete attempt at writing a better version of am_hal_adc_read_samples:

// // Definition of a function to unload from the ADC DM/FIFO/Other Buffer with concern for the slot number
// am_hal_status_e unloadADC   (  
//                             void*       pHandle,        // Handle to the ADC module
//                             uint32_t*   psource,
//                             uint32_t    numSource,      // NUmber of elements in the source
//                             uint8_t*    pslots,         // Pointer to an array of slot numbers that go into each number
//                             uint32_t**  pbuffs,         // Pointer to an array of pointers to the buffers to place results into
//                             uint32_t*   plengths,       // Pointer to array of lengths of the buffers
//                             uint8_t     num             // The number of buffer definitions that exist (read: the number of elements in 'pslots', 'pbuffs', and 'pbuffs')
//                             ){

//         // uint32_t      ui32Sample;
//         // uint32_t      ui32RequestedSamples = *pui32InOutNumberSamples;

//     // uint32_t ui32Module = ((am_hal_adc_state_t*)pHandle)->ui32Module; // We don't actually need this until we implement FIFO or bufferless DMA 

// #ifndef AM_HAL_DISABLE_API_VALIDATION
//     // if ( !AM_HAL_ADC_CHK_HANDLE(pHandle) ){return AM_HAL_STATUS_INVALID_HANDLE;}    // Check the handle.
//     if(((pHandle) && ((am_hal_handle_prefix_t *)(pHandle))->s.bInit && (((am_hal_handle_prefix_t *)(pHandle))->s.magic == 0xAFAFAF))){return AM_HAL_STATUS_INVALID_HANDLE;}
//     // if ( NULL == pui32OutBuffer ){return AM_HAL_STATUS_INVALID_ARG;}                // Check the output sample buffer pointer.
// #endif

//     if( psource == NULL ){return AM_HAL_STATUS_INVALID_ARG;}
//     if( pslots == NULL ){return AM_HAL_STATUS_INVALID_ARG;}
//     if( pbuffs == NULL ){return AM_HAL_STATUS_INVALID_ARG;}
//     if( plengths == NULL ){return AM_HAL_STATUS_INVALID_ARG;}
//     if( num == 0 ){return AM_HAL_STATUS_INVALID_ARG;}
//     if( numSource == 0 ){return AM_HAL_STATUS_INVALID_ARG;}

//     for(uint8_t indi = 0; indi < num; indi++){
//         if( *(pbuffs + indi) == 0){return AM_HAL_STATUS_INVALID_ARG;}
//     }

//     const uint8_t maxNumSlots = 8;
//     if( num > maxNumSlots ){return AM_HAL_STATUS_INVALID_ARG;}

//     uint32_t offsets[maxNumSlots];
//     memset(offsets, 0, maxNumSlots*sizeof(uint32_t));
    

//     // *pui32InOutNumberSamples = 0;

//     //
//     // Check if we are reading directly from FIFO or DMA SRAM buffer.
//     //
//     if ( NULL == psource )  // Grab a value from the ADC FIFO
//     {                   
//         // do
//         // {
//         //     ui32Sample = ADCn(ui32Module)->FIFOPR;
//         //     pui32OutBuffer->ui32Slot   = AM_HAL_ADC_FIFO_SLOT(ui32Sample);
//         //     pui32OutBuffer->ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(ui32Sample);
//         //     pui32OutBuffer++;
//         //     (*pui32InOutNumberSamples)++;
//         // } while ((AM_HAL_ADC_FIFO_COUNT(ui32Sample) > 0) &&
//         //          (*pui32InOutNumberSamples < ui32RequestedSamples));
//         return AM_HAL_STATUS_INVALID_ARG;   // Not yet implemented
//     }
//     else
//     {           // Process the samples from the provided sample buffer

//         while(numSource > 0){

//             am_hal_adc_sample_t local_sample;                               // Get this sample
//             local_sample.ui32Slot = AM_HAL_ADC_FIFO_SLOT(*psource);
//             local_sample.ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(*psource);

//             for(uint8_t indi = 0; indi < num; indi++){                      // Now see if it fits in any of our buffers. Sadly I think this will just chuck any samples that are associated with slots that we don't care about
//                 uint8_t wanted_slot = *( pslots + indi );   // This is the current slot that we are trying to see if the sample can go into
//                 if( wanted_slot == local_sample.ui32Slot ){

//                     (*(pbuffs + indi) + offsets[indi]) = 
//                     offsets[indi]++;

//                 }
//                 // else do nothing
//             }


//             psource++;
//             numSource--;
//         }

//         // do
//         // {
//         //     ui32Sample = ADCn(ui32Module)->FIFOPR;
//         //     pui32OutBuffer->ui32Slot   = AM_HAL_ADC_FIFO_SLOT(*pui32InSampleBuffer);
//         //     pui32OutBuffer->ui32Sample = AM_HAL_ADC_FIFO_SAMPLE(*pui32InSampleBuffer);
//         //     pui32InSampleBuffer++;
//         //     pui32OutBuffer++;
//         //     (*pui32InOutNumberSamples)++;
//         // } while (*pui32InOutNumberSamples < ui32RequestedSamples);
//     }
//     return AM_HAL_STATUS_SUCCESS;   // Return FIFO valid bits.
// }