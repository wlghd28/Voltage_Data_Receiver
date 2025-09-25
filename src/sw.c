/*
 * sw.c
 *
 *  Created on: Sep 23, 2024
 *      Author: User
 */

// include
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"
#include "tim.h"
#include "adc.h"
#include "spi.h"
#include "sw.h"
#include "hw.h"
#include "master.h"
#include "ltc6806.h"

// declare
STC_SW_GLOBAL_VAR sw_global_var;

void SW_SPI1_RecvCheckBusy();
void SW_SPI1_SendCheckBusy();

// define
void InitProc()
{
	// variable
	memset(&sw_global_var, 0, sizeof(sw_global_var));

	Init_LTC6806();
	Init_Master();

	// interrupt
	if(HAL_TIM_Base_Start_IT(&htim1) != HAL_OK){}
	if(HAL_UART_Receive_IT(&huart1, sw_global_var.UART1_recvBuf, sizeof(sw_global_var.UART1_recvBuf)) != HAL_OK){}
}

void ProcMain()
{
	SetLED(sw_global_var.LED_flag.Bits.LED0);
	switch(sw_global_var.ProcState)
	{
	case PROC_STATE_NONE:
		sw_global_var.ProcState = PROC_STATE_INIT;
		break;

	case PROC_STATE_INIT:
		sw_global_var.SPI1_recvCheckBusy = 1;
		sw_global_var.SPI1_sendCheckBusy = 1;
		SetSEN(RESET);
		//Broadcast_WriteData_LTC6806(LTC6806_WRCFG);
		Address_WriteData_LTC6806(LTC6806_WRCFG);
		SW_SPI1_SendCheckBusy();
		HAL_SPI_Receive_IT(&hspi1, sw_global_var.SPI1_recvBuf, sizeof(sw_global_var.SPI1_recvBuf));
		SW_SPI1_RecvCheckBusy();
		SetSEN(SET);
		sw_global_var.ProcState = PROC_STATE_LTC6806_ADCV;
		break;

	case PROC_STATE_LTC6806_ADCV:
		SetSEN(RESET);
		//Broadcast_StartCellVoltageADC_LTC6806();
		Address_StartCellVoltageADC_LTC6806();
		SW_SPI1_SendCheckBusy();
		HAL_SPI_Receive_IT(&hspi1, sw_global_var.SPI1_recvBuf, sizeof(sw_global_var.SPI1_recvBuf));
		SW_SPI1_RecvCheckBusy();
		SetSEN(SET);
		ltc6806_global_var.Cmd = LTC6806_RDCVA;
		sw_global_var.ProcState = PROC_STATE_LTC6806_RDCV;
		break;

	case PROC_STATE_LTC6806_RDCV:
		SetSEN(RESET);
		//Broadcast_ReadData_LTC6806(ltc6806_global_var.Cmd);
		Address_ReadData_LTC6806(ltc6806_global_var.Cmd);
		SW_SPI1_SendCheckBusy();
		HAL_SPI_Receive_IT(&hspi1, sw_global_var.SPI1_recvBuf, sizeof(sw_global_var.SPI1_recvBuf));
		SW_SPI1_RecvCheckBusy();
		SetSEN(SET);
		ProcComBuf_LTC6806(ltc6806_global_var.Cmd);
		ltc6806_global_var.Cmd++;
		sw_global_var.ProcState = ltc6806_global_var.Cmd > LTC6806_RDCVI ? PROC_STATE_LTC6806_ADCV : PROC_STATE_MASTER;
		break;

	case PROC_STATE_MASTER:
		ProcComBuf_Master();
		sw_global_var.ProcState = PROC_STATE_LTC6806_RDCV;
		break;

	default:
		break;
	}
}

void ProcTim1()
{
	if(++sw_global_var.TIM1_nTmr >= 50)
	{
		sw_global_var.LED_flag.Bits.LED0 ^= 1;
		sw_global_var.TIM1_nTmr = 0;
	}
}

void SW_SPI1_RecvCheckBusy()
{
	while(sw_global_var.SPI1_recvCheckBusy){HAL_Delay(1);}
	sw_global_var.SPI1_recvCheckBusy = 1;
}

void SW_SPI1_SendCheckBusy()
{
	while(sw_global_var.SPI1_sendCheckBusy){HAL_Delay(1);}
	sw_global_var.SPI1_sendCheckBusy = 1;
}

// callback override
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	if(huart->Instance == USART1)
	{
		RecvComBuf_Master(sw_global_var.UART1_recvBuf, sizeof(sw_global_var.UART1_recvBuf));
		HAL_UART_Receive_IT(huart, sw_global_var.UART1_recvBuf, sizeof(sw_global_var.UART1_recvBuf));
		return;
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == SPI1)
	{
		sw_global_var.SPI1_recvCheckBusy = 0;
		return;
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * hspi)
{
	if(hspi->Instance == SPI1)
	{
		sw_global_var.SPI1_sendCheckBusy = 0;
		return;
	}
}

