/*
 * ltc6806.h
 *
 *  Created on: Sep 23, 2024
 *      Author: User
 */

#ifndef INC_LTC6806_H_
#define INC_LTC6806_H_

#include "stm32f1xx_hal.h"

#define LTC6806_SENDDATA_LEN 12	// byte
#define LTC6806_RECVDATA_LEN 8	// byte
#define LTC6806_CHANNEL_LEN 36

// enum
enum	// command
{
	LTC6806_WRCFG,			// 0x01
	LTC6806_RDCFG,			// 0x02
	LTC6806_RDCVA,			// 0x04
	LTC6806_RDCVB,			// 0x05
	LTC6806_RDCVC,			// 0x06
	LTC6806_RDCVD,			// 0x07
	LTC6806_RDCVE,			// 0x08
	LTC6806_RDCVF,			// 0x09
	LTC6806_RDCVG,			// 0x0A
	LTC6806_RDCVH,			// 0x0B
	LTC6806_RDCVI,			// 0x0C
	LTC6806_RDAUXA,			// 0x10
	LTC6806_RDAUXB,			// 0x11
	LTC6806_RDSTATA,		// 0x14
	LTC6806_RDSTATB,		// 0x15
	LTC6806_RDSTATC,		// 0x16
	LTC6806_ADCV,			// 0x440
	LTC6806_CMD_LEN
};

// struct, union
#pragma pack(1)
typedef union
{
	int16_t Val;
	struct
	{
		uint16_t CC0 : 1;
		uint16_t CC1 : 1;
		uint16_t CC2 : 1;
		uint16_t CC3 : 1;
		uint16_t CC4 : 1;
		uint16_t CC5 : 1;
		uint16_t CC6 : 1;
		uint16_t CC7 : 1;
		uint16_t CC8 : 1;
		uint16_t CC9 : 1;
		uint16_t CC10 : 1;
		uint16_t a0 : 1;
		uint16_t a1 : 1;
		uint16_t a2 : 1;
		uint16_t a3 : 1;
		uint16_t MSB : 1;
	} Bits;
}UNI_LTC6806_CMD;

typedef union
{
	uint16_t Val;
	struct
	{
		uint16_t B0 : 1;
		uint16_t B1 : 1;
		uint16_t B2 : 1;
		uint16_t B3 : 1;
		uint16_t B4 : 1;
		uint16_t B5 : 1;
		uint16_t B6 : 1;
		uint16_t B7 : 1;
		uint16_t B8 : 1;
		uint16_t B9 : 1;
		uint16_t B10 : 1;
		uint16_t B11 : 1;
		uint16_t B12 : 1;
		uint16_t B13 : 1;
		uint16_t B14 : 1;
		uint16_t B15 : 1;
	} Bits;
}UNI_LTC6806_PEC;

typedef struct
{
	uint16_t Cmd;
	UNI_LTC6806_CMD Broadcast_cmdBuf[LTC6806_CMD_LEN];
	UNI_LTC6806_CMD Address_cmdBuf[LTC6806_CMD_LEN];
	UNI_LTC6806_PEC Broadcast_pecBuf[LTC6806_CMD_LEN];
	UNI_LTC6806_PEC Address_pecBuf[LTC6806_CMD_LEN];
	int16_t pec15Table[256];
	uint8_t sendBuf[LTC6806_SENDDATA_LEN];
	uint8_t recvBuf[LTC6806_CMD_LEN][LTC6806_RECVDATA_LEN];
	int16_t Volt[LTC6806_CHANNEL_LEN];		// mV
}STC_LTC6806_GLOBAL_VAR;
#pragma pack()

// extern
extern STC_LTC6806_GLOBAL_VAR ltc6806_global_var;

extern void Init_LTC6806();
extern void RecvComBuf_LTC6806(uint8_t* Buf, uint32_t Len);
extern void ProcComBuf_LTC6806(uint16_t cmd);
extern void Broadcast_StartCellVoltageADC_LTC6806();
extern void Broadcast_ReadData_LTC6806(uint16_t cmd);
extern void Broadcast_WriteData_LTC6806(uint16_t cmd);
extern void Address_StartCellVoltageADC_LTC6806();
extern void Address_ReadData_LTC6806(uint16_t cmd);
extern void Address_WriteData_LTC6806(uint16_t cmd);


#endif /* INC_LTC6806_H_ */
