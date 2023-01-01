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
/*
  NatureDSP Signal Processing Library. IIR part
    Lattice Block Real IIR, 32x32-bit, M=3
    C code optimized for HiFi4
  IntegrIT, 2006-2014
*/
#include "NatureDSP_types.h"
#include "common.h"
#include "latr32x32_common.h"

/*-------------------------------------------------------------------------
  Lattice Block Real IIR
  Computes a real cascaded lattice autoregressive IIR filter using reflection 
  coefficients stored in vector k. The real data input are stored in vector x.
  The filter output result is stored in vector r.  Input scaling is done before 
  the first cascade for normalization and overflow protection..

  Precision: 
  32x32  32-bit data, 32-bit coefficients
  32x16  32-bit data, 16-bit coefficients

  Input:
  N      length of input sample block
  M      number of reflection coefficients
  scale  input scale factor g, Q31 or Q15
  k[M]   reflection coefficients, Q31 or Q15
  x[N]   input samples, Q31
  Output:
  r[N]   output data, Q31

  Restriction:
  x,r,k should not overlap

  PERFORMANCE NOTE:
  for optimum performance follow rules:
  M - from the range 1...8
-------------------------------------------------------------------------*/
void latr32x32_3_proc(       int32_t * restrict r,     // r[N]     [out   ] Q31
                       const int32_t * restrict x,     // x[N]     [in    ] Q31
                             int32_t * restrict delLine, // dline[M] [in/out] Q30
                       const int32_t *  restrict coef,  // coef[M]  [in    ] Q31
                             int32_t scale, int N, int M )  // scale    [in    ] Q31
{
    const ae_int32* restrict px=(const ae_int32*)x;
          ae_int32* restrict pr=(      ae_int32*)r;
    ae_int32x2 s,c01,c12,c23,xn,rn;
    ae_f32x2 d23,d01,d12,d_0,t0;
    ae_f64 q1,q0;
    int n;

    NASSERT(M==3);
    NASSERT_ALIGN8(coef);
    NASSERT_ALIGN8(delLine);
    rn=0;
    c01=AE_L32X2_I((const ae_int32x2*)coef,0);
    c23=AE_L32X2_I((const ae_int32x2*)coef,8);
    c12=AE_SEL32_LH(c01,c23);

    d01=AE_L32X2_I((const ae_int32x2*)delLine,0);
    d23=AE_L32X2_I((const ae_int32x2*)delLine,8);
    d12=AE_SEL32_LH(d01,d23);
    d_0=AE_SEL32_LH(  0,d01);
    s=AE_MOVDA32(scale>>1);
    __Pragma("loop_count min=1")
    for ( n=0; n<N; n++ )
    {
        AE_L32_IP(xn,px,4);
        q0=AE_MULF32S_LL(xn,s);
        AE_MULSSFD32S_HH_LL(q0,d12,c12);
        q1=q0;
        AE_MULSF32S_LH(q1,d_0,c01);
        t0=AE_ROUND32X2F64SASYM(q1,q0);
        d12=AE_SEL32_LH(d_0,d12);
        AE_MULAFP32X2RAS(d12,t0,c01);
        d_0=AE_ROUND32X2F64SASYM(q1,q1);
        rn=AE_TRUNCI32F64S_L(rn,q1,1);  // Make the output sample.
        AE_S32_L_IP(rn,pr,4);
    }
    d23=AE_SEL32_LH(d12,0);
    d01=AE_SEL32_LH(d_0,d12);
    AE_S32X2_I(d01,(ae_int32x2*)delLine,0);
    AE_S32X2_I(d23,(ae_int32x2*)delLine,8);
}
