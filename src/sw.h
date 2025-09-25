/*
 * sw.h
 *
 *  Created on: Sep 23, 2024
 *      Author: User
 */

#ifndef INC_SW_H_
#define INC_SW_H_

#include "stm32f1xx_hal.h"

#define SW_UART1_RECVDATA_LEN 1
#define SW_SPI1_RECVDATA_LEN 8

// enum
enum
{
	PROC_STATE_NONE,
	PROC_STATE_INIT,
	PROC_STATE_LTC6806_ADCV,
	PROC_STATE_LTC6806_RDCV,
	PROC_STATE_MASTER,
	PROC_STATE_LEN
};

// struct, union
#pragma pack(1)
typedef struct
{
	uint8_t Val;
	struct
	{
		uint8_t LED0 : 1;
		uint8_t b1 : 1;
		uint8_t b2 : 1;
		uint8_t b3 : 1;
		uint8_t b4 : 1;
		uint8_t b5 : 1;
		uint8_t b6 : 1;
		uint8_t b7 : 1;
	} Bits;
}UNI_SW_LED;

typedef struct
{
	uint8_t ProcState;
	UNI_SW_LED LED_flag;
	uint8_t TIM1_nTmr;
	uint8_t UART1_recvBuf[SW_UART1_RECVDATA_LEN];
	uint8_t SPI1_recvBuf[SW_SPI1_RECVDATA_LEN];
	uint8_t SPI1_recvCheckBusy;	// 0 : ready, 1 : busy
	uint8_t SPI1_sendCheckBusy;	// 0 : ready, 1 : busy
}STC_SW_GLOBAL_VAR;
#pragma pack()

// extern
extern STC_SW_GLOBAL_VAR sw_global_var;

extern void InitProc();
extern void ProcMain();
extern void ProcTim1();

#endif /* INC_SW_H_ */
