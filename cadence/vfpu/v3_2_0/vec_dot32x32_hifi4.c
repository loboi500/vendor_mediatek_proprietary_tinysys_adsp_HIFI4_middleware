/*
 *    Mediatek HiFi 4 Redistribution Version  <0.0.1>
 */

/* ------------------------------------------------------------------------ */
/* Copyright (c) 2016 by Cadence Design Systems, Inc. ALL RIGHTS RESERVED.  */
/* These coded instructions, statements, and computer programs (�Cadence    */
/* Libraries�) are the copyrighted works of Cadence Design Systems Inc.	    */
/* Cadence IP is licensed for use with Cadence processor cores only and     */
/* must not be used for any other processors and platforms. Your use of the */
/* Cadence Libraries is subject to the terms of the license agreement you   */
/* have entered into with Cadence Design Systems, or a sublicense granted   */
/* to you by a direct Cadence licensee.                                     */
/* ------------------------------------------------------------------------ */
/*  IntegrIT, Ltd.   www.integrIT.com, info@integrIT.com                    */
/*                                                                          */
/* DSP Library                                                              */
/*                                                                          */
/* This library contains copyrighted materials, trade secrets and other     */
/* proprietary information of IntegrIT, Ltd. This software is licensed for  */
/* use with Cadence processor cores only and must not be used for any other */
/* processors and platforms. The license to use these sources was given to  */
/* Cadence, Inc. under Terms and Condition of a Software License Agreement  */
/* between Cadence, Inc. and IntegrIT, Ltd.                                 */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2015-2016 IntegrIT, Limited.                      */
/*                      All Rights Reserved.                                */
/* ------------------------------------------------------------------------ */
#include "NatureDSP_Signal.h"
#include "common.h"


/*===========================================================================
  Vector matematics:
  vec_dot              Vector Dot Product
===========================================================================*/

/*-------------------------------------------------------------------------
  Vector Dot product
  Two versions of routines are available: regular versions (without suffix _fast)
  that work with arbitrary arguments, faster versions 
  (with suffix _fast) apply some restrictions.

  Precision: 
  64x32  64x32-bit data, 64-bit output (fractional multiply Q63xQ31->Q63)
  64x64  64x64-bit data, 64-bit output (fractional multiply Q63xQ63->Q63)
  64x64i 64x64-bit data, 64-bit output (low 64 bit of integer multiply)
  32x32  32x32-bit data, 64-bit output
  32x16  32x16-bit data, 64-bit output
  16x16  16x16-bit data, 64-bit output for regular version and 32-bit for 
                        fast version
  f      single precision floating point

  Input:
  x[N]  input data, Q31 or floating point
  y[N]  input data, Q31, Q15, or floating point
  N	length of vectors
  Returns: dot product of all data pairs, Q31 or Q63

  Restrictions:
  Regular versions (without suffix _fast):
  None
  Faster versions (with suffix _fast):
  x,y - aligned on 8-byte boundary
  N   - multiple of 4

  vec_dot16x16_fast utilizes 32-bit saturating accumulator, so, input data 
  should be scaled properly to avoid erroneous results
-------------------------------------------------------------------------*/
int64_t vec_dot32x32 (const int32_t * restrict x,const int32_t * restrict y,int N)
{
  int n;
  ae_int32x2 y0,z0,z2,z1;
  ae_int64  Acc;
  ae_f64 t=AE_MOVINT64_FROMINT32X2(AE_MOVDA32X2(0,0x8000));
  
  const ae_int32x2 *  restrict  py = (const ae_int32x2 *)  (y);
  const ae_int32x2 *  restrict  pz = (const ae_int32x2 *)  (x);
                                         
  ae_valign y_align;
  if(N<=0) return 0;

  if (((uintptr_t)pz)&4)
  {
      AE_L32_IP(z1, castxcc(ae_int32,pz), 4);
      y_align = AE_LA64_PP(py);
      for (n=0; n<((N&(~1))>>1); n++)
      {
        AE_L32X2_IP(z0, pz, 8);
        z2=AE_SEL32_LH(z1,z0);
        z1=z0;
        AE_LA32X2_IP(y0, y_align,py);
        AE_MULAAFD32RA_HH_LL(t,z2,y0);
      }
      if(N&1)
      {
        AE_LA32X2_IP(y0, y_align,py);
        AE_MULAF32RA_LH(t,z1,y0);
      }
      Acc=AE_SRAI64(t,16);
  }
  else
  {
      y_align = AE_LA64_PP(py);
        AE_LA32X2_IP(y0, y_align,py);
      for (n=0; n<((N&(~1))>>1); n++)
      {
        AE_L32X2_IP (z0, pz, 8);
        AE_MULAAFD32RA_HH_LL(t,z0,y0);
        AE_LA32X2_IP(y0, y_align,py);
      }
      if(N&1)
      {
        AE_L32_IP(z0, castxcc(ae_int32,pz), 4);
        AE_MULAF32RA_HH(t,z0,y0);
      }
      Acc=AE_SRAI64(t,16);
  }

  return Acc;
}