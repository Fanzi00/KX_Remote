#ifndef __USER_CONFIG_H

#include <stdint.h>

#include "app_lcd.h"
#include "LPC17xx.h"
#include "config.h"


#define KXCAN_GLGJT312E_L3X 1
#define KXCAN_GLZYTC_L2X 2

/*******************USER_TYPE*******************/
/*******************USER_TYPE*******************/

#define USER_TYPE 			KXCAN_GLGJT312E_L3X

/*******************USER_TYPE*******************/
/*******************USER_TYPE*******************/

// 河南耿力 拱架台车 312E
#if (USER_TYPE == KXCAN_GLGJT312E_L3X)
#include "KXCAN_GLGJT312E.h"


#endif // #if (USER_TYPE == KXCAN_GLGJT312E_L3X)




#endif // #ifndef __USER_CONFIG_H
