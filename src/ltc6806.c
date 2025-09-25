/*
 * ltc6806.c
 *
 *  Created on: Sep 23, 2024
 *      Author: User
 */

// include
#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "sw.h"
#include "spi.h"
#include "ltc6806.h"
#include "queue.h"

// declare
STC_LTC6806_GLOBAL_VAR ltc6806_global_var;

void Init_Broadcast_cmdBuf_LTC6806();
void Init_Broadcast_pecBuf_LTC6806();
void Init_Address_cmdBuf_LTC6806();
void Init_Address_pecBuf_LTC6806();
void Init_PEC15Table_LTC6806();
uint16_t Cal_PEC15_LTC6806(char *data, int len);
int16_t ConvertADCToVolt_LTC6806(uint16_t val);

// define
void Init_LTC6806()
{
	memset(&ltc6806_global_var, 0, sizeof(ltc6806_global_var));

	Init_PEC15Table_LTC6806();
	Init_Broadcast_cmdBuf_LTC6806();
	Init_Broadcast_pecBuf_LTC6806();
	Init_Address_cmdBuf_LTC6806();
	Init_Address_pecBuf_LTC6806();
}


void RecvComBuf_LTC6806(uint8_t* Buf, uint32_t Len)
{
	for(int i = 0; i < Len; i++)
	{
		Queue_Insert(&Queue_Ltc6806, Buf[i]);
	}
}

void ProcComBuf_LTC6806(uint16_t cmd)
{
	uint8_t i = cmd;
	uint16_t recv_pec = 0, cal_pec = 0;

	memcpy(ltc6806_global_var.recvBuf[i], sw_global_var.SPI1_recvBuf, sizeof(sw_global_var.SPI1_recvBuf));

	// Check PEC
	memcpy(&recv_pec, ltc6806_global_var.recvBuf[i] + (LTC6806_RECVDATA_LEN - sizeof(recv_pec)), sizeof(recv_pec));
	cal_pec = Cal_PEC15_LTC6806((char*)ltc6806_global_var.recvBuf[i], (int)(LTC6806_RECVDATA_LEN - sizeof(recv_pec)));
	cal_pec = __REV16(cal_pec);

	if(recv_pec == cal_pec)
	{
		// Data Parsing
		uint8_t channel_index = i - LTC6806_RDCVA;
		uint16_t adc_buf[2];

		// channel 1, 5, 9, 13, 17, 21, 25, 29, 33
		memset(adc_buf, 0, sizeof(adc_buf));
		adc_buf[0] = ltc6806_global_var.recvBuf[i][0] << 4;
		adc_buf[1] = ltc6806_global_var.recvBuf[i][1] >> 4;
		ltc6806_global_var.Volt[channel_index * 4] = ConvertADCToVolt_LTC6806(adc_buf[0] | adc_buf[1]);

		// channel 2, 6, 10, 14, 18, 22, 26, 30, 34
		memset(adc_buf, 0, sizeof(adc_buf));
		adc_buf[0] = ltc6806_global_var.recvBuf[i][1] << 12; adc_buf[0] = adc_buf[0] >> 4;
		adc_buf[1] = ltc6806_global_var.recvBuf[i][2];
		ltc6806_global_var.Volt[channel_index * 4 + 1] = ConvertADCToVolt_LTC6806(adc_buf[0] | adc_buf[1]);

		// channel 3, 7, 11, 15, 19, 23, 27, 31, 35
		memset(adc_buf, 0, sizeof(adc_buf));
		adc_buf[0] = ltc6806_global_var.recvBuf[i][3] << 4;
		adc_buf[1] = ltc6806_global_var.recvBuf[i][4] >> 4;
		ltc6806_global_var.Volt[channel_index * 4 + 2] = ConvertADCToVolt_LTC6806(adc_buf[0] | adc_buf[1]);

		// channel 4, 8, 12, 16, 20, 24, 28, 32, 36
		memset(adc_buf, 0, sizeof(adc_buf));
		adc_buf[0] = ltc6806_global_var.recvBuf[i][4] << 12; adc_buf[0] = adc_buf[0] >> 4;
		adc_buf[1] = ltc6806_global_var.recvBuf[i][5];
		ltc6806_global_var.Volt[channel_index * 4 + 3] = ConvertADCToVolt_LTC6806(adc_buf[0] | adc_buf[1]);
	}
}

void Broadcast_StartCellVoltageADC_LTC6806()
{
	int16_t l_cmd = 0;
	uint16_t l_pec = 0;
	uint16_t size = 0;

	l_cmd = __REV16(ltc6806_global_var.Broadcast_cmdBuf[LTC6806_ADCV].Val);
	l_pec = __REV16(ltc6806_global_var.Broadcast_pecBuf[LTC6806_ADCV].Val);

	memset(ltc6806_global_var.sendBuf, 0, sizeof(ltc6806_global_var.sendBuf));
	memcpy(ltc6806_global_var.sendBuf, &l_cmd, sizeof(l_cmd));
	size += sizeof(l_cmd);
	memcpy(ltc6806_global_var.sendBuf + sizeof(l_cmd), &l_pec, sizeof(l_pec));
	size += sizeof(l_pec);

	HAL_SPI_Transmit_IT(&hspi1, (const uint8_t*)ltc6806_global_var.sendBuf, (uint16_t)size);
}

void Broadcast_ReadData_LTC6806(uint16_t cmd)
{
	int16_t l_cmd = 0;
	uint16_t l_pec = 0;
	uint16_t size = 0;

	l_cmd = __REV16(ltc6806_global_var.Broadcast_cmdBuf[cmd].Val);
	l_pec = __REV16(ltc6806_global_var.Broadcast_pecBuf[cmd].Val);

	memset(ltc6806_global_var.sendBuf, 0, sizeof(ltc6806_global_var.sendBuf));
	memcpy(ltc6806_global_var.sendBuf, &l_cmd, sizeof(l_cmd));
	size += sizeof(l_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec, sizeof(l_pec));
	size += sizeof(l_pec);

	HAL_SPI_Transmit_IT(&hspi1, (const uint8_t*)ltc6806_global_var.sendBuf, (uint16_t)size);
}

void Broadcast_WriteData_LTC6806(uint16_t cmd)
{
	int16_t l_cmd = 0;
	uint8_t data[6] = { 0, };
	uint16_t l_pec_cmd = 0, l_pec_data = 0;
	uint16_t size = 0;

	l_cmd = __REV16(ltc6806_global_var.Broadcast_cmdBuf[cmd].Val);
	l_pec_cmd = __REV16(ltc6806_global_var.Broadcast_pecBuf[cmd].Val);

	data[0] = 63;	// RSVD, GPIO1~GPIO6
	data[1] = 0;	// HIRING, REFON, OWPCH[1]~OWPCH[0], REV[3]~REV[0]
	data[2] = 0;	// MMD[1]~MMD[0], FCHNL[5]~FCHNL[0]
	data[3] = 0;	// VUV[11]~VUV[4]
	data[4] = 0;	// VUV[3]~VUV[0], VOV[11]~VOV[8]
	data[5] = 0;	// VOV[7]~VOV[0]

	l_pec_data = Cal_PEC15_LTC6806((char*)data, sizeof(data));
	l_pec_data = __REV16(l_pec_data);

	memset(ltc6806_global_var.sendBuf, 0, sizeof(ltc6806_global_var.sendBuf));
	memcpy(ltc6806_global_var.sendBuf, &l_cmd, sizeof(l_cmd));
	size += sizeof(l_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec_cmd, sizeof(l_pec_cmd));
	size += sizeof(l_pec_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, data, sizeof(data));
	size += sizeof(data);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec_data, sizeof(l_pec_data));
	size += sizeof(l_pec_data);

	HAL_SPI_Transmit_IT(&hspi1, (const uint8_t*)ltc6806_global_var.sendBuf, (uint16_t)size);
}

void Address_StartCellVoltageADC_LTC6806()
{
	int16_t l_cmd = 0;
	uint16_t l_pec = 0;
	uint16_t size = 0;

	l_cmd = __REV16(ltc6806_global_var.Address_cmdBuf[LTC6806_ADCV].Val);
	l_pec = __REV16(ltc6806_global_var.Address_pecBuf[LTC6806_ADCV].Val);

	memset(ltc6806_global_var.sendBuf, 0, sizeof(ltc6806_global_var.sendBuf));
	memcpy(ltc6806_global_var.sendBuf, &l_cmd, sizeof(l_cmd));
	size += sizeof(l_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec, sizeof(l_pec));
	size += sizeof(l_pec);

	HAL_SPI_Transmit_IT(&hspi1, (const uint8_t*)ltc6806_global_var.sendBuf, (uint16_t)size);
}

void Address_ReadData_LTC6806(uint16_t cmd)
{
	int16_t l_cmd = 0;
	uint16_t l_pec = 0;
	uint16_t size = 0;

	l_cmd = __REV16(ltc6806_global_var.Address_cmdBuf[cmd].Val);
	l_pec = __REV16(ltc6806_global_var.Address_pecBuf[cmd].Val);

	memset(ltc6806_global_var.sendBuf, 0, sizeof(ltc6806_global_var.sendBuf));
	memcpy(ltc6806_global_var.sendBuf, &l_cmd, sizeof(l_cmd));
	size += sizeof(l_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec, sizeof(l_pec));
	size += sizeof(l_pec);

	HAL_SPI_Transmit_IT(&hspi1, (const uint8_t*)ltc6806_global_var.sendBuf, (uint16_t)size);
}

void Address_WriteData_LTC6806(uint16_t cmd)
{
	int16_t l_cmd = 0;
	uint8_t data[6] = { 0, };
	uint16_t l_pec_cmd = 0, l_pec_data = 0;
	uint16_t size = 0;

	l_cmd = __REV16(ltc6806_global_var.Address_cmdBuf[cmd].Val);
	l_pec_cmd = __REV16(ltc6806_global_var.Address_pecBuf[cmd].Val);

	data[0] = 63;	// RSVD, GPIO1~GPIO6
	data[1] = 0;	// HIRING, REFON, OWPCH[1]~OWPCH[0], REV[3]~REV[0]
	data[2] = 0;	// MMD[1]~MMD[0], FCHNL[5]~FCHNL[0]
	data[3] = 0;	// VUV[11]~VUV[4]
	data[4] = 0;	// VUV[3]~VUV[0], VOV[11]~VOV[8]
	data[5] = 0;	// VOV[7]~VOV[0]

	l_pec_data = Cal_PEC15_LTC6806((char*)data, sizeof(data));
	l_pec_data = __REV16(l_pec_data);

	memset(ltc6806_global_var.sendBuf, 0, sizeof(ltc6806_global_var.sendBuf));
	memcpy(ltc6806_global_var.sendBuf, &l_cmd, sizeof(l_cmd));
	size += sizeof(l_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec_cmd, sizeof(l_pec_cmd));
	size += sizeof(l_pec_cmd);
	memcpy(ltc6806_global_var.sendBuf + size, data, sizeof(data));
	size += sizeof(data);
	memcpy(ltc6806_global_var.sendBuf + size, &l_pec_data, sizeof(l_pec_data));
	size += sizeof(l_pec_data);

	HAL_SPI_Transmit_IT(&hspi1, (const uint8_t*)ltc6806_global_var.sendBuf, (uint16_t)size);
}

void Init_Broadcast_cmdBuf_LTC6806()
{
	uint16_t addr = 0;
	uint16_t arrBits[LTC6806_CMD_LEN] = { 0x01, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x10, 0x11, 0x14, 0x15, 0x16, 0x440 };

	for(int i = 0; i < LTC6806_CMD_LEN; i++)
	{
		ltc6806_global_var.Broadcast_cmdBuf[i].Val += 0;
		ltc6806_global_var.Broadcast_cmdBuf[i].Val = ltc6806_global_var.Broadcast_cmdBuf[i].Val << 4;
		ltc6806_global_var.Broadcast_cmdBuf[i].Val += addr;
		ltc6806_global_var.Broadcast_cmdBuf[i].Val = ltc6806_global_var.Broadcast_cmdBuf[i].Val << 11;
		ltc6806_global_var.Broadcast_cmdBuf[i].Val += arrBits[i];
	}
}

void Init_Broadcast_pecBuf_LTC6806()
{
	int16_t l_cmd = 0;
	for(int i = 0; i < LTC6806_CMD_LEN; i++)
	{
		l_cmd = __REV16(ltc6806_global_var.Broadcast_cmdBuf[i].Val);
		ltc6806_global_var.Broadcast_pecBuf[i].Val = Cal_PEC15_LTC6806((char*)&l_cmd, sizeof(l_cmd));
	}
}

void Init_Address_cmdBuf_LTC6806()
{
	uint16_t addr = 0;
	uint16_t arrBits[LTC6806_CMD_LEN] = { 0x01, 0x02, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x10, 0x11, 0x14, 0x15, 0x16, 0x440 };

	for(int i = 0; i < LTC6806_CMD_LEN; i++)
	{
		ltc6806_global_var.Address_cmdBuf[i].Val += 1;
		ltc6806_global_var.Address_cmdBuf[i].Val = ltc6806_global_var.Address_cmdBuf[i].Val << 4;
		ltc6806_global_var.Address_cmdBuf[i].Val += addr;
		ltc6806_global_var.Address_cmdBuf[i].Val = ltc6806_global_var.Address_cmdBuf[i].Val << 11;
		ltc6806_global_var.Address_cmdBuf[i].Val += arrBits[i];
	}
}

void Init_Address_pecBuf_LTC6806()
{
	int16_t l_cmd = 0;
	for(int i = 0; i < LTC6806_CMD_LEN; i++)
	{
		l_cmd = __REV16(ltc6806_global_var.Address_cmdBuf[i].Val);
		ltc6806_global_var.Address_pecBuf[i].Val = Cal_PEC15_LTC6806((char*)&l_cmd, sizeof(l_cmd));
	}
}

void Init_PEC15Table_LTC6806()
{
	int16_t CRC15_POLY = 0x4599;
	int16_t remainder = 0;

	for (int i = 0; i < 256; i++)
	{
		remainder = i << 7;
		for (int bit = 8; bit > 0; --bit)
		{
			if (remainder & 0x4000)
			{
				remainder = ((remainder << 1));
				remainder = (remainder ^ CRC15_POLY);
			}
			else
			{
				remainder = ((remainder << 1));
			}
		}
		ltc6806_global_var.pec15Table[i] = remainder & 0xFFFF;
	}
}

uint16_t Cal_PEC15_LTC6806(char *data, int len)
{
	int16_t remainder,address;
	remainder = 16;	// PEC seed

	for (int i = 0; i < len; i++)
	{
		address = ((remainder >> 7) ^ data[i]) & 0xff;	// calculate PEC table address
		remainder = (remainder << 8 ) ^ ltc6806_global_var.pec15Table[address];
	}

	return (remainder * 2);	// The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}

int16_t ConvertADCToVolt_LTC6806(uint16_t val)
{
	int16_t ret_val = 0;
	int16_t adc_val = val;

	if(adc_val & 0x800)					// 부호 bit (12번째 bit) 판별 시 음수일 경우
	{
#if 0
		adc_val = adc_val ^ 0xFFF;		// 12 bit XOR연산 	>> 1의보수
		adc_val += 	1;					// 1의보수 결과 값 + 1 	>> 2의보수
		adc_val *= -1;
#else
		adc_val = adc_val | 0xF000;		// 13 ~ 16 번째 bit 1로 채운다.
#endif
	}
	ret_val = (int16_t)((double)adc_val * (double)(1.5));	// cell resolution = 1.5 (HIRNG = 0)
															// cell resolution = 3 	 (HIRNG = 1)
	return ret_val;
}
