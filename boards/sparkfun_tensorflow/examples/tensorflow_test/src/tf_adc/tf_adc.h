
#ifndef TF_ADC_H
#define TF_ADC_H

#define ADC_EXAMPLE_DEBUG 1
#define ADC_SAMPLE_RATE 10

int initADC(void);
void triggerAdc(void);
void enableAdcInterrupts(void);

#endif /* TF_ADC_H */