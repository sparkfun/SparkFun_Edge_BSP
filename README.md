# SparkFun_TensorFlow_Apollo3_BSP
BSP and examples to integrate the SparkFun TensorFlow board with the AmbiqSuite

## Installation

Clone or download and extract this repo into the ```SDK/boards/``` directory. 

## Examples
This repo contains two example projects. 
* In 'edge_test' the hardware features of the board are demonstrated and the makefile is preconfigured with relative paths.
  * To compile with gcc and flash use the makefile in the gcc subdirectory. 
    * Edit 'COM_PORT' variable to specify where to try UART flashing
    * make
    * make clean
    * make bootload
* The 'SparkFun_Edge_Project_Template' has a relatively easy to set up makefile with some example header files and source files included. You can copy this directory to an arbitraty location on your filesystem to begin a new project. 
  * You must provide the absolute path to the SDK root directory in the 'SDKPATH' variable
  * Also update the COM_PORT variable to match your setup
