/*****************************************************************************
 *   config.h:  Type definition Header file for NXP LPC17xx Family 
 *   Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.25  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include <stdint.h>

#ifndef __CONFIG_H__
#define __CONFIG_H__


#include "math.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"


#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif


#define  Fextal		(12000000UL)		// 外部晶振的频率
//#define  Fcclk  (12000000UL)


//typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;
//typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;


//#define PWM_CYCLE		300
//#define  LPCREG		unsigned int


/*
#include "..\ZY_CODE\zy_if\zy_if.h"
#include "..\cfg_file\target_cfg.h"
#include "common\inc\lpc17xx.h"
//#include "common\inc\lpc1700.h"
 */

#include <lpc17xx.h>
#include "data_types.h"
#include "time.h"
#include "can.h"
#include "timer.h"
#include "UART.h"
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "i2c2.h"
//#include "pwm.h"
#include "canopen.h"
#include "wl.h"

#include "APP_BUS.h"
#include "APP_IO.h"
#include "APP_DGUS.h"
#include "APP_lcd.h"
#include "APP_usr.h"
#include "app_comm.h"



void delay_ms(unsigned int vDel);


#endif  /* __TYPE_H__ */
