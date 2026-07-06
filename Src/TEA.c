/*******************
  TEA加密解密算法
*******************/

#include "TEA.h"


#define MX (z>>5^y<<2)+(y>>3^z<<4)^(sum^y)+(k[p&3^e]^z)
#define DELTA 0x9e3779b9
#define S_LOOPTIME   1    //5
#define BLOCK_SIZE   8//31   //PAGE_SIZE,根据你所要加密的数据包长度修改此参数(单位:字节)

/* 
*key  maybe 128bit =16 Bytes.
*buf  maybe BLOCK_SIZE
*/


void btea_encrypt( unsigned char* buf, unsigned char len, unsigned char* key )
{
  unsigned char  n= len/4;//BLOCK_SIZE/4;
  unsigned long *v=(unsigned long *)buf;
  unsigned long *k=(unsigned long *)key;
  unsigned long z = v[n - 1],y = v[0],sum = 0,e ;
  unsigned char p,q ;
  // Coding Part 

  //n = len/4;
  q = S_LOOPTIME + 52 / n ;
  while ( q-- > 0 )
  {
    sum += DELTA ;
    e = sum >> 2 & 3 ;
    for ( p = 0 ; p < n - 1 ; p++ )
    {
      y = v[p + 1];
      z = v[p] += MX;
    }
      y = v[0] ;
    z = v[n - 1] += MX;
  }
}


/*
*key  maybe 128bit =16Bytes.
*buf  maybe BLOCK_SIZE
inbuf == outbuf == buf
*/


void btea_decrpyt( unsigned char* buf, unsigned char len, unsigned char* key )
{
  unsigned char n= len/4;//BLOCK_SIZE/4;
  unsigned long *v=(unsigned long *)buf;
  unsigned long *k=(unsigned long *)key;
  unsigned long z = v[n - 1],y = v[0],sum = 0,e ;
  unsigned char  p,q ;

  //Decoding Part...
  //n = len/4;
  q = S_LOOPTIME + 52 / n ;
  sum = q * DELTA ;
  while ( sum != 0 )
  {
    e = sum >> 2 & 3 ;
    for ( p = n - 1 ; p > 0 ; p-- )
    {
      z = v[p - 1];
      y = v[p] -= MX;
    }
      z = v[n - 1] ;
    y = v[0] -= MX;
            sum -= DELTA ;
  }
}
