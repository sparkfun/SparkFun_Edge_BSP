# SparkFun_TensorFlow_Apollo3_BSP
BSP and examples to integrate the SparkFun TensorFlow board with the AmbiqSuite

## Installation

Clone or download then extract or symlink this repo into the ```SDK/boards/``` directory. 

## Examples
This repo contains several example projects. 
* **example1_edge_test** demonstrates the hardware features of the board. The makefile is preconfigured with relative paths.
  * To compile with gcc and flash use the makefile in the gcc subdirectory. 
    * Edit 'COM_PORT' variable to specify where to try UART flashing
    * make
    * make clean
    * make bootload
* **example2_dual_mic_dma** shows how to use DMA to capture 16 kHz audio recordings from both microphones without blocking the processor. This is the basis of how audio is provided to the TensorFlow Lite demo.
* **tensorflow_demo** uses a pre-trained model to identify "yes" and "no" and blink a corresponding LED on the board. Using GPIO you could easily expand this example to control a device. By following the [SparkFun Edge Guide to TensorFlow Lite](https://learn.sparkfun.com/tutorials/sparkfun-edge-guide-to-tensorflow-lite) you can change the keywords to "on" and "off."
* **SparkFun_Edge_Project_Template** has a relatively easy to set up makefile with some example header files and source files included. You can copy this directory to an arbitraty location on your filesystem to begin a new project. 
  * You must provide the absolute path to the SDK root directory in the 'SDKPATH' variable
  * Also update the COM_PORT variable to match your setup
