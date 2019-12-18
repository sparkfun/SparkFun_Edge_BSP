#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "tf_adc.h"
#include "am_util.h"

static void init_timerA1_for_ADC(void);
static void adc_config(void);
static void adc_deconfig(void);

volatile uint32_t audioSample = 0;

// ADC Device Handle.
static void *g_ADCHandle;

// // ADC Pin
// const am_hal_gpio_pincfg_t g_AM_PIN_29_ADCSE1 =
// {
//     .uFuncSel       = AM_HAL_PIN_29_ADCSE1,
// };
const am_hal_gpio_pincfg_t g_AM_PIN_11_ADCSE2 =
{
    .uFuncSel       = AM_HAL_PIN_11_ADCSE2,
};

void
am_ctimer_isr(void)
{
  // Clear TimerA0 Interrupt.
  am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

  // Re-configure the ADC. We lose configuation data in the power-down, so
  // we'll reconfigure the ADC here. If you don't shut down the ADC, this
  // step is unnecessary.
  adc_config();

  // Trigger the ADC
  am_hal_adc_sw_trigger(g_ADCHandle);
}

void am_adc_isr(void)
{
    uint32_t            ui32IntMask;
    am_hal_adc_sample_t Sample;

    // Read the interrupt status.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading ADC interrupt status\n");
    }

    // Clear the ADC interrupt.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing ADC interrupt status\n");
    }
    // If we got a conversion completion interrupt (which should be our only
    // ADC interrupt), go ahead and read the data.
    if (ui32IntMask & AM_HAL_ADC_INT_CNVCMP)
    {
        uint32_t    ui32NumSamples = 1;
        if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle,
                                                         true,
                                                         NULL,
                                                         &ui32NumSamples,
                                                         &Sample))
        {
        am_util_stdio_printf("Error - ADC sample read from FIFO failed.\n");
    }

#if (1 == ADC_EXAMPLE_DEBUG)
    // am_util_stdio_printf("ADC: %d = %d\n", Sample.ui32Slot, Sample.ui32Sample);
    am_util_stdio_printf("%d\n", Sample.ui32Sample);
#endif
  }

    audioSample = Sample.ui32Sample;

    adc_deconfig();
    
    am_hal_adc_sw_trigger(g_ADCHandle);
}

int initADC(void)
{
    // am_hal_gpio_pinconfig(29, g_AM_PIN_29_ADCSE1);
    am_hal_gpio_pinconfig(11, g_AM_PIN_11_ADCSE2);

    init_timerA1_for_ADC();

    return 0;
}

static void adc_config(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    // Initialize the ADC and get the handle.
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the ADC instance failed.\n");
    }

    // Power on the ADC.
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                            AM_HAL_SYSCTRL_WAKE,
                                                            false) )
    {
        am_util_stdio_printf("Error - ADC power on failed.\n");
    }

    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
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

    // Set up an ADC slot
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_14BIT;
    // ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE1;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE2;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 0 failed.\n");
    }
  
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_CNVCMP );

    //
    // Enable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC failed.\n");
    }
}

static void adc_deconfig(void)
{
  //
  // Disable the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_disable(g_ADCHandle))
  {
    am_util_stdio_printf("Error - disable ADC failed.\n");
  }

  //
  // Enable the ADC power domain.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC))
  {
    am_util_stdio_printf("Error - disabling the ADC power domain failed.\n");
  }

  //
  // Initialize the ADC and get the handle.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_deinitialize(g_ADCHandle))
  {
    am_util_stdio_printf("Error - return of the ADC instance failed.\n");
  }

}

void triggerAdc(void)
{
    am_hal_adc_sw_trigger(g_ADCHandle);
}

void enableAdcInterrupts(void)
{
    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_EnableIRQ(CTIMER_IRQn);
    am_hal_interrupt_master_enable();
}

static void init_timerA1_for_ADC(void)
{
    // Start a timer to trigger the ADC periodically. This timer won't actually
    // be connected to the ADC (as can be done with Timer 3). Instead, we'll
    // generate interrupts to the CPU, and then use the CPU to trigger the ADC
    // in the CTIMER interrupt handler.
    am_hal_ctimer_config_single(0, AM_HAL_CTIMER_TIMERA,
                                AM_HAL_CTIMER_LFRC_512HZ |
                                    AM_HAL_CTIMER_FN_REPEAT |
                                    AM_HAL_CTIMER_INT_ENABLE);
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, ADC_SAMPLE_RATE, 0);
    
    // Start the timer
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
}