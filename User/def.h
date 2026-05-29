/*
 * board_type.h
 *
 *  Created on: 2025Äê1ÔÂ13ÈÕ
 *      Author: Reed
 */

#ifndef USER_DEF_H_
#define USER_DEF_H_

//#define USE_ADV_COR
#define USE_COROUTINE

#ifdef USE_ADV_COR
#undef USE_COROUTINE
#endif

#define portINTR interrupt("WCH-Interrupt-fast")
#ifdef USE_ADV_COR
#undef portINTR
#define portINTR interrupt()
#endif

#define COMPILE_WL

#endif /* USER_DEF_H_ */
