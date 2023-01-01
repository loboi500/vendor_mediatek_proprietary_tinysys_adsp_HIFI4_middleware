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
int64_t vec_dot64x64_fast (const int64_t * restrict x,const int64_t * restrict y,int N)
{
  int n;
  ae_int32x2 y0, x0, y1, x1;
  ae_int64 xw0, yw0;                                     
  ae_int64 W0,  W1, ACC;
  ae_ep e0,e1;
  const ae_int64 *  restrict  px = (const ae_int64 *) x;
  const ae_int64 *  restrict  py = (const ae_int64 *) y;
  NASSERT_ALIGN8(x);
  NASSERT_ALIGN8(y);
  NASSERT((N&3)==0);
  if(N<=0) return 0;
  e0=e1=AE_MOVEA(0); 
  W0=W1=AE_ZERO64();
  __Pragma("loop_count min=2")
  for (n=0; n<(N>>1); n++)
  {
        xw0=AE_L64_I ( px,  8); x1 = AE_MOVINT32X2_FROMINT64(xw0);
        yw0=AE_L64_I ( py,  8); y1 = AE_MOVINT32X2_FROMINT64(yw0);
        AE_L64_IP(xw0, px, 16); x0 = AE_MOVINT32X2_FROMINT64(xw0);
        AE_L64_IP(yw0, py, 16); y0 = AE_MOVINT32X2_FROMINT64(yw0);
        //  Q63xQ63->Q63   
        AE_MULAAD32USEP_HL_LH (e0,W0,x0,y0);
        AE_MULAAD32USEP_HL_LH (e0,W0,x1,y1);
#if 0
        x0=AE_SEL32_HH(x0,x1);
        y0=AE_SEL32_HH(y0,y1);
        AE_MULAAD32EP_HH_LL(e1,W1,x0,y0);
#else
        AE_MULA32EP_HH(e1,W1,x0,y0);
        AE_MULA32EP_HH(e1,W1,x1,y1);
#endif
  }
    AE_ADD72(e1,W1,e1,W1);
    W0 = AE_SRAI72(e0,W0,31);
    AE_ADD72X64(e1,W1,W0);
    ACC = AE_SAT64S(e1,W1);
    return ACC;
}