/******************************************************************************
*
* FILE: tmuflang.h
*
*------------------------------------------------------------------------------
*
*  Copyright 2004, 2005 Trend Micro, Inc.  All rights reserved
*
******************************************************************************/

#ifndef _TMUF_LANG_H_
#define _TMUF_LANG_H_

#include <string.h>
#include "emprimitive.h"

/**
 * This data type is used to be as a 64-bits bit map to indicate in what languages
 * users are going to import URLs from the pattern file.
 * This also points out the maximum languages we support : 64.
 */
typedef em_uint32 TM_UF_LANG_MAP [2];


/*===========================================================================*
 *                     Language Map Operation Macros                         *
 *===========================================================================*/
/**
 * This macro initializes the language map. MUST be called before using a language map.
 *
 * @param LMAP                    (in)The language map.
 */
#define TM_UF_INIT_LANG_MAP(LMAP)       (memset((LMAP), 0, sizeof(TM_UF_LANG_MAP)))

/**
 * This macro adds an language into a language map.
 *
 * @param LID                     (in)The language ID.
 * @param LMAP                    (in)The language map.
 */
#define TM_UF_ADD_LANG(LID, LMAP)       (((LID) >= 0 && (LID) < 64) ? ((LMAP)[(LID) >> 5] |= 1 << ((LID) & 31)) : 0)

/**
 * This macro removes an language from a language map.
 *
 * @param LID                     (in)The language ID.
 * @param LMAP                    (in)The language map.
 */
#define TM_UF_REMOVE_LANG(LID, LMAP)    (((LID) >= 0 && (LID) < 64) ? ((LMAP)[(LID) >> 5] &= ~(1 << ((LID) & 31))) : 0)

/**
 * This macro checks if the specified language is enabled in the language map.
 *
 * @param LID                     (in)The language ID.
 * @param LMAP                    (in)The language map.
 */
#define TM_UF_IS_LANG_ENABLE(LID, LMAP) (((LID) >= 0 && (LID) < 64) ? ((LMAP)[(LID) >> 5] & (1 << ((LID) & 31))) : FALSE)

/*===========================================================================*
 *                        Language ID Definitions                            *
 *===========================================================================*/
#define TM_UF_LANG_ALL                0
#define TM_UF_LANG_ENGLISH            1
/***
  .
  .
 Still need further definition.
  .
  .
*****/
#define TM_UF_LANG_MAX           64  /* IDs can not exceed this boundary */


#endif /* _TMUF_LANG_H_ */
