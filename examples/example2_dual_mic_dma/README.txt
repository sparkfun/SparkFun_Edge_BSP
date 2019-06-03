This example shows the CTIMER-A3 triggering repeated 16 KHz samples
based on an external input at 12MHz in LPMODE0.  The example uses the
CTIMER-A3 to trigger ADC sampling.  Each data point is transferred
from the ADC FIFO into an SRAM buffer using DMA. In the code you will
also find hints on how to enable 128 sample average and other options.