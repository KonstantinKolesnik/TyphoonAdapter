#ifndef ACKDETECTOR_H_
#define ACKDETECTOR_H_
//---------------------------------------------------------------------------------------------
void SetProgPhase(uint8_t isProg);
void ResetAckFlag();
uint8_t QueryAckFlag();
void CheckAcknowledgement();
//---------------------------------------------------------------------------------------------
#endif