#ifndef _EDGE_CAMERA_H_
#define _EDGE_CAMERA_H_

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include <string.h>


// void edge_cam_start_clkbuf_xfer( void );

uint32_t cameraSetup( void );

// interrupt handlers
void edge_cam_isr( uint64_t ui64IntMask );
void edge_cam_int_pclk( void );
void edge_cam_int_href( void );
void edge_cam_int_vsync( void );

#endif /* _EDGE_CAMERA_H_ */