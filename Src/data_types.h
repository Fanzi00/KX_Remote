/***********************************************************************
 * File:   data_types.h
 * Rev:    1.0
 * Author: Chun Sing Chu
 * Date:   March 26 2007
 *
 * Description:
 *     This file contains the typedefs for C standard types.
 *
 * Revision History:
 * Rev 1.0 March 26 2007
 * Initial revision.
 * 
 **********************************************************************/

#ifndef __DATA_TYPES_H
#define __DATA_TYPES_H

/***********************************************************************
 * Data  types definition                                  
 **********************************************************************/
typedef unsigned char   BOOL;
typedef unsigned char * pBOOL;
typedef unsigned char   INT8U;
typedef unsigned char * pINT8U;
typedef signed   char   INT8S;
typedef signed   char * pINT8S;
typedef unsigned short  INT16U;
typedef unsigned short* pINT16U;
typedef signed   short  INT16S;
typedef signed   short* pINT16S;
typedef unsigned int    INT32U;
typedef unsigned int  * pINT32U;
typedef signed   int    INT32S;
typedef signed   int  * pINT32S;
typedef unsigned char   uint8;
typedef unsigned long   ulong;

typedef unsigned short WORD;
typedef unsigned long  DWORD;
//typedef unsigned int   BOOL;
typedef unsigned int   uint;
typedef unsigned short uint16;
typedef unsigned long  uint32;
typedef unsigned short ushort;
typedef unsigned char  uchar;



#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#define _NO_ERROR          (INT32S)(0)
#define _ERROR             (INT32S)(-1)



#endif /* __DATA_TYPES_H */


