#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include "ProgramQueue.h"
//---------------------------------------------------------------------------------------------
#define PROGRAM_QUEUE_SIZE 10//5
static Message_t progQueue[PROGRAM_QUEUE_SIZE];
static uint8_t progWriteIdx = 0;
static volatile uint8_t progReadIdx = 0;
//---------------------------------------------------------------------------------------------
static bool IsFull();
static bool IsEmpty();
//---------------------------------------------------------------------------------------------
void AddToProgramQueue(Message_t* msg)
{
	if (!IsFull())
	{
		memcpy((void*)&progQueue[progWriteIdx], msg, sizeof(Message_t));
		progWriteIdx++;
		progWriteIdx %= PROGRAM_QUEUE_SIZE;
	}
}
Message_t* GetFromProgramQueue()
{
    if (!IsEmpty())
    {
		uint8_t idx = progReadIdx;
        progReadIdx++;
        progReadIdx %= PROGRAM_QUEUE_SIZE;
		
		return (Message_t*)&progQueue[idx];
    }
	else
		return NULL;
}
void ClearProgramQueue()
{
	progReadIdx = progWriteIdx = 0;
}
static bool IsFull()
{
    return (((progWriteIdx + 1) % PROGRAM_QUEUE_SIZE) == progReadIdx);
}
static bool IsEmpty()
{
    return (progReadIdx == progWriteIdx);
}
//---------------------------------------------------------------------------------------------
