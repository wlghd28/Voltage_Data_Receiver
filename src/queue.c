/*
 * queue.c
 *
 *  Created on: Sep 23, 2024
 *      Author: User
 */

#include "queue.h"

QUEUE Queue_Master;
QUEUE Queue_Ltc6806;

void Queue_Insert(QUEUE *Queue, uint8_t recvbuf)
{

	Queue->Buffer[Queue->Head++] = recvbuf;
	Queue->Head &= (QUEUE_RX_BUFSIZE - 1);
	if(Queue->Head == Queue->Tail)
	{
		Queue->Tail++;
		Queue->Tail &= (QUEUE_RX_BUFSIZE - 1);
	}
}

void Queue_Delete(QUEUE *Queue, uint16_t size)
{
	Queue->Tail += size;
	Queue->Tail &= (QUEUE_RX_BUFSIZE - 1);
}

uint8_t Queue_Getdata(QUEUE *Queue, uint16_t index)
{
	uint16_t lindex = index;
	lindex &= (QUEUE_RX_BUFSIZE - 1);
	return Queue->Buffer[lindex];
}

uint16_t Queue_Getdatalength(QUEUE *Queue)
{
	uint16_t ret = 0;
	if(Queue->Head >= Queue->Tail) ret = Queue->Head - Queue->Tail;
	else ret = QUEUE_RX_BUFSIZE - Queue->Tail + Queue->Head;
	return ret;
}

uint16_t Queue_Gethead(QUEUE *Queue)
{
	return Queue->Head;
}

uint16_t Queue_Gettail(QUEUE *Queue)
{
	return Queue->Tail;
}

