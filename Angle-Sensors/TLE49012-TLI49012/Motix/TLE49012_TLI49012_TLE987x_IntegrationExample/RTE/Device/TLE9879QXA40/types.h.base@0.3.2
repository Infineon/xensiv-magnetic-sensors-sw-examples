/*
 ***********************************************************************************************************************
 *
 * Copyright (c) Infineon Technologies AG
 * All rights reserved.
 *
 * The applicable license agreement can be found at this pack's installation directory in the file
 * license/IFX_SW_Licence_MOTIX_LITIX.txt
 *
 **********************************************************************************************************************/
/**
 * \file     types.h
 *
 * \brief    General type declarations
 *
 * \version  V0.3.2
 * \date     02. Jan 2025
 *
 */

/*******************************************************************************
**                             Author(s) Identity                             **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** SS           Steffen Storandt                                              **
** DM           Daniel Mysliwitz                                              **
** JO           Julia Ott                                                     **
** BG           Blandine Guillot                                              **
** PS           Patrik Schwarz                                                **
** VO           Vanessa Ongaro                                                **
*******************************************************************************/

/*******************************************************************************
**                          Revision Control History                          **
********************************************************************************
** V0.1.0: 2012-11-12, SS:   Initial version                                  **
** V0.2.0: 2012-12-13, SS:   Formal changes                                   **
** V0.2.1: 2017-11-13, DM:   Register access macros added                     **
** V0.2.2: 2018-02-14, DM:   MISRA 2012 compliance, the following PC-Lint     **
**                           rules are globally deactivated:                  **
**                           - Info 793: ANSI/ISO limit of 6 'significant     **
**                             characters in an external identifier           **
**                           - Info 835: A zero has been given as right       **
**                             argument to operator                           **
**                           - Info 845: The left argument to operator '&'    **
**                             is certain to be 0                             **
**                           Field_Rd1() return typecase changed to (uint8)   **
** V0.2.3: 2019-03-11, JO:   Changed 'typedef signed long long int64' to      **
**                           'typedef signed long long sint64'                **
** V0.2.4: 2019-09-12, JO:   For ARM GCC compatibility:                       **
**                           - Added #include <stdint.h>                      **
**                           - Use intx_t for defining sintx                  **
**                           - Use uintx_t for defining uint                  **
** V0.2.5: 2020-04-15, BG:   Updated revision history format                  **
** V0.2.6: 2020-07-21, BG:   EP-439: Formatted .h/.c files                    **
** V0.2.7: 2020-07-28, JO:   EP-431: Checked if __CC_ARM is defined before    **
**                           checking its value                               **
** V0.2.8: 2021-06-30, BG:   EP-855: Added "defined" in condition for IAR     **
** V0.2.9: 2022-02-28, JO:   EP-936: Updated copyright and branding           **
** V0.3.0: 2022-10-18, PS:   EP-1153: Replaced boolean type with bool         **
** V0.3.1: 2022-10-18, VO:   EP-1252: Removed ARMCC V6.18 warnings,           **
**                                    Added __inline for C90 support          **
** V0.3.2: 2025-01-02, JO:   EP-1493: Updated license                         **
*******************************************************************************/

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/*******************************************************************************
**                                  Includes                                  **
*******************************************************************************/

/*******************************************************************************
**                          Global Macro Definitions                          **
*******************************************************************************/
/** \brief Standard real type definitions to ease the use of different types
 */
#define STD_REAL_FIX_POINT_16 (1)
#define STD_REAL_FIX_POINT_32 (2)
#define STD_REAL_FLOAT_32     (3)

#ifndef STD_REAL_TYPE
  #define STD_REAL_TYPE (STD_REAL_FIX_POINT_16)
#endif

/** \brief Boolean definitions
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
  #include <stdbool.h>
#else
  #define bool unsigned int
  #define true 1
  #define false 0
#endif

/** \brief Shift definitions for fix point format
 */
#define STD_REAL_SHIFT_Qn8_y (-8  - 1) /**< \brief Range=[-2^(-9), 2^(-9)[; Q1.15=>Step=1/2^24, Q1.31 =>Step=1/2^40 */
#define STD_REAL_SHIFT_Qn7_y (-7  - 1) /**< \brief Range=[-2^(-8), 2^(-8)[; Q1.15=>Step=1/2^23, Q1.31 =>Step=1/2^39 */
#define STD_REAL_SHIFT_Qn6_y (-6  - 1) /**< \brief Range=[-2^(-7), 2^(-7)[; Q1.15=>Step=1/2^22, Q1.31 =>Step=1/2^38 */
#define STD_REAL_SHIFT_Qn5_y (-5  - 1) /**< \brief Range=[-2^(-6), 2^(-6)[; Q1.15=>Step=1/2^21, Q1.31 =>Step=1/2^37 */
#define STD_REAL_SHIFT_Qn4_y (-4  - 1) /**< \brief Range=[-2^(-5), 2^(-5)[; Q1.15=>Step=1/2^20, Q1.31 =>Step=1/2^36 */
#define STD_REAL_SHIFT_Qn3_y (-3  - 1) /**< \brief Range=[-2^(-4), 2^(-4)[; Q1.15=>Step=1/2^19, Q1.31 =>Step=1/2^35 */
#define STD_REAL_SHIFT_Qn2_y (-2  - 1) /**< \brief Range=[-2^(-3), 2^(-3)[; Q1.15=>Step=1/2^18, Q1.31 =>Step=1/2^34 */
#define STD_REAL_SHIFT_Qn1_y (-1  - 1) /**< \brief Range=[-2^(-2), 2^(-2)[; Q1.15=>Step=1/2^17, Q1.31 =>Step=1/2^33 */

#define STD_REAL_SHIFT_Q0_y  ( 0  - 1) /**< \brief Range=[-2^(-1), 2^(-1)[; Q1.15=>Step=1/2^16, Q1.31 =>Step=1/2^32 */
#define STD_REAL_SHIFT_Q1_y  ( 1  - 1) /**< \brief Range=[-2^0   , 2^0[;    Q1.15=>Step=1/2^15, Q1.31 =>Step=1/2^31 */
#define STD_REAL_SHIFT_Q2_y  ( 2  - 1) /**< \brief Range=[-2^1   , 2^1[;    Q1.15=>Step=1/2^14, Q1.31 =>Step=1/2^30 */
#define STD_REAL_SHIFT_Q3_y  ( 3  - 1) /**< \brief Range=[-2^2   , 2^2[;    Q1.15=>Step=1/2^13, Q1.31 =>Step=1/2^29 */
#define STD_REAL_SHIFT_Q4_y  ( 4  - 1) /**< \brief Range=[-2^3   , 2^3[;    Q1.15=>Step=1/2^12, Q1.31 =>Step=1/2^28 */
#define STD_REAL_SHIFT_Q5_y  ( 5  - 1) /**< \brief Range=[-2^4   , 2^4[;    Q1.15=>Step=1/2^11, Q1.31 =>Step=1/2^27 */
#define STD_REAL_SHIFT_Q6_y  ( 6  - 1) /**< \brief Range=[-2^5   , 2^5[;    Q1.15=>Step=1/2^10, Q1.31 =>Step=1/2^26 */
#define STD_REAL_SHIFT_Q7_y  ( 7  - 1) /**< \brief Range=[-2^6   , 2^6[;    Q7.9 =>Step=1/2^9,  Q8.24 =>Step=1/2^25 */
#define STD_REAL_SHIFT_Q8_y  ( 8  - 1) /**< \brief Range=[-2^7   , 2^7[;    Q8.8 =>Step=1/2^8,  Q8.24 =>Step=1/2^24 */
#define STD_REAL_SHIFT_Q9_y  ( 9  - 1) /**< \brief Range=[-2^8   , 2^8[;    Q9.7 =>Step=1/2^7,  Q9.23 =>Step=1/2^23 */
#define STD_REAL_SHIFT_Q10_y ( 10 - 1) /**< \brief Range=[-2^9   , 2^9[;    Q10.6=>Step=1/2^6,  Q10.22=>Step=1/2^22 */
#define STD_REAL_SHIFT_Q11_y ( 11 - 1) /**< \brief Range=[-2^10  , 2^10[;   Q11.5=>Step=1/2^5,  Q11.21=>Step=1/2^21 */
#define STD_REAL_SHIFT_Q12_y ( 12 - 1) /**< \brief Range=[-2^11  , 2^11[;   Q12.4=>Step=1/2^4,  Q12.20=>Step=1/2^20 */
#define STD_REAL_SHIFT_Q13_y ( 13 - 1) /**< \brief Range=[-2^12  , 2^12[;   Q13.3=>Step=1/2^3,  Q13.19=>Step=1/2^19 */
#define STD_REAL_SHIFT_Q14_y ( 14 - 1) /**< \brief Range=[-2^13  , 2^13[;   Q14.2=>Step=1/2^2,  Q14.18=>Step=1/2^18 */
#define STD_REAL_SHIFT_Q15_y ( 15 - 1) /**< \brief Range=[-2^14  , 2^14[;   Q15.1=>Step=1/2^1,  Q15.17=>Step=1/2^17 */
#define STD_REAL_SHIFT_Q16_y ( 16 - 1) /**< \brief Range=[-2^15  , 2^15[;   Q16.0=>Step=1,      Q16.16=>Step=1/2^16 */
#define STD_REAL_SHIFT_Q32_y ( 32 - 1) /**< \brief Range=[-2^31  , 2^31[;   Q16.0=>             Q32.0 =>Step=1      */

#define NULL_PTR ((void*)0)

#define INLINE __STATIC_INLINE

/*******************************************************************************
**                           Global Type Definitions                          **
*******************************************************************************/
typedef uint8_t uint8;           /**< \brief 8 bit unsigned value  */
typedef uint16_t uint16;         /**< \brief 16 bit unsigned value */
typedef uint32_t uint32;           /**< \brief 32 bit unsigned value */
typedef uint64_t uint64;     /**< \brief 64 bit unsigned value */

typedef int8_t sint8;             /**< \brief 8 bit signed value  */
typedef int16_t sint16;           /**< \brief 16 bit signed value */
typedef int32_t sint32;             /**< \brief 32 bit signed value */
typedef int64_t sint64;       /**< \brief 64 bit signed value */

typedef float   float32;               /**< \brief 32 bit float value */
typedef double  float64;               /**< \brief 64 bit float value */

typedef short FixPoint16;              /**< \brief 16 bit fix point value */
typedef long  FixPoint32;              /**< \brief 32 bit fix point value */


/** \brief Standard real type definitions to ease the use of different types
 */
#if (STD_REAL_TYPE == STD_REAL_FIX_POINT_16)
  typedef FixPoint16 TStdReal;
  typedef FixPoint32 TLongStdReal;
#elif (STD_REAL_TYPE == STD_REAL_FIX_POINT_32)
  typedef FixPoint32 TStdReal;
  typedef FixPoint32 TLongStdReal;
#elif (STD_REAL_TYPE == STD_REAL_FLOAT_32)
  typedef float32 TStdReal;
  typedef float32 TLongStdReal;
#endif


/** \brief Complex type definition based on TStdReal
 */
typedef struct StdRealComplex
{
  TStdReal imag;   /**< \brief Imaginary part */
  TStdReal real;   /**< \brief Real part */
} TStdRealComplex;

/** \brief Complex type definition
 */
typedef struct Complex
{
  sint16 Real;  /**< \brief Real part */
  sint16 Imag;  /**< \brief Imaginary part */
} TComplex;

/** \brief 2 phase currents type definition
 */
typedef struct PhaseCurr
{
  sint16 A;   /**< \brief Phase A current */
  sint16 B;   /**< \brief Phase B current */
} TPhaseCurr;

#endif /* TYPES_H */

