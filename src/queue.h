/*
 * queue.h
 *
 *  Created on: Sep 23, 2024
 *      Author: User
 */

#ifndef INC_QUEUE_H_
#define INC_QUEUE_H_

#include <stdint.h>

#define QUEUE_RX_BUFSIZE					512		// Max Rx Buffer size

#pragma pack(1)
typedef struct
{
	uint16_t Head, Tail;
	uint8_t Buffer[QUEUE_RX_BUFSIZE];
}QUEUE;
#pragma pack()

extern QUEUE Queue_Master;
extern QUEUE Queue_Ltc6806;

extern void Queue_Insert(QUEUE *Queue, uint8_t recvbuf);
extern void Queue_Delete(QUEUE *Queue, uint16_t size);
extern uint8_t Queue_Getdata(QUEUE *Queue, uint16_t index);
extern uint16_t Queue_Getdatalength(QUEUE *Queue);
extern uint16_t Queue_Gethead(QUEUE *Queue);
extern uint16_t Queue_Gettail(QUEUE *Queue);


#endif /* INC_QUEUE_H_ */
