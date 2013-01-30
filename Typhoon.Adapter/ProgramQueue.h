#ifndef PROGRAMQUEUE_H_
#define PROGRAMQUEUE_H_
//---------------------------------------------------------------------------------------------
#include "DCCMessage.h"
//---------------------------------------------------------------------------------------------
void AddToProgramQueue(Message_t* msg);
Message_t* GetFromProgramQueue();
void ClearProgramQueue();
//---------------------------------------------------------------------------------------------
#endif