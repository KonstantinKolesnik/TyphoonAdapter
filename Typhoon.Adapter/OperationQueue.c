#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include "OperationQueue.h"
//---------------------------------------------------------------------------------------------
#define OPERATION_QUEUE_SIZE 20
static Message_t operQueue[OPERATION_QUEUE_SIZE];
static uint8_t operWriteIdx = 0;
static volatile uint8_t operReadIdx = 0;
//---------------------------------------------------------------------------------------------
static bool IsFull();
static bool IsEmpty();
//---------------------------------------------------------------------------------------------
void AddToOperationQueue(Message_t *msg)
{
	if (!IsFull())
	{
		memcpy((void*)&operQueue[operWriteIdx], msg, sizeof(Message_t));
		operWriteIdx++;
		operWriteIdx %= OPERATION_QUEUE_SIZE;
	}
}
Message_t* GetFromOperationQueue()
{
    if (!IsEmpty())
    {
		uint8_t idx = operReadIdx;
        operReadIdx++;
        operReadIdx %= OPERATION_QUEUE_SIZE;
		
		return (Message_t*)&operQueue[idx];
    }
	else
		return NULL;
}
static bool IsFull()
{
    return (((operWriteIdx + 1) % OPERATION_QUEUE_SIZE) == operReadIdx);
}
static bool IsEmpty()
{
    return (operReadIdx == operWriteIdx);
}
//---------------------------------------------------------------------------------------------
