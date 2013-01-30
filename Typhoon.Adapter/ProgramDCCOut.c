#include "Hardware.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "DCCMessage.h"
#include "ProgramQueue.h"
#include "AckDetector.h"
//---------------------------------------------------------------------------------------------
volatile uint8_t Repeats = 0;
static Message_t DCCIdle;
static DCCOutputInfo doi;
//---------------------------------------------------------------------------------------------
static uint8_t IsServiceProgCommand(Message_t *msg);
static void SetNextMessage(Message_t *msg);
static inline void SetBit(uint8_t bit) __attribute__((always_inline));
//---------------------------------------------------------------------------------------------
// DCC generator Interface
//---------------------------------------------------------------------------------------------
static uint8_t IsServiceProgCommand(Message_t *pMsg)
{
	// 0111CCAA AAAAAAAA DDDDDDDD - CV
	// 0111CCAA AAAAAAAA 111KDBBB - bit
	
	if (pMsg == NULL)
		return 0;
		
	uint8_t serviceMask = 0b01110000;
	
	if ((pMsg->Dcc[0] & serviceMask) == serviceMask)
		return 1;
	else
		return 0;
}
static void SetNextMessage(Message_t *pMsg)
{
	memcpy(doi.Dcc, pMsg->Dcc, pMsg->Size);
	doi.Size = pMsg->Size;
	Repeats = pMsg->Repeats;
}
static void SetBit(uint8_t bit)
{
	TCCR3A = (1<<COM3A1) | (0<<COM3A0)  // pin OC3A = "0" on compare match (DCC_PROG)
           | (1<<COM3B1) | (1<<COM3B0)  // pin OC3B = "1" on compare match (NDCC_PROG)
		   | (0<<FOC3A)  | (0<<FOC3B)
           | (0<<WGM31)  | (0<<WGM30);

	OCR3A = OCR3B = F_CPU * (bit == 0 ? PERIOD_0 : PERIOD_1) / 2 / 1000000L; // 1392/696 ticks
}
void InitProgramDCCOut()
{
	DCCIdle.Size = 2;
	DCCIdle.Repeats = 1;
    DCCIdle.Dcc[0] = 255;
    DCCIdle.Dcc[1] = 0;
	
    doi.State = Idle;

	// set Timer/Counter3
    TCNT3 = 0; // no prescaler
    
	TCCR3A = (1<<COM3A1) | (0<<COM3A0)  // pin OC3A = "0" on compare match (DCC_PROG)
           | (1<<COM3B1) | (1<<COM3B0)  // pin OC3B = "1" on compare match (NDCC_PROG)
		   | (0<<FOC3A)  | (0<<FOC3B)	// reserved in PWM, set to zero
           | (0<<WGM31)  | (0<<WGM30);  // (+ WGM32, WGM33) CTC mode, TOP = OCR3A
    
	TCCR3B = (0<<ICNC3)  | (0<<ICES3)   // Input Capture Noise Canceler off; Input Capture Edge Select off
           | (0<<WGM33)  | (1<<WGM32)	// (+ WGM30, WGM30) CTC mode, TOP = OCR3A
           | (0<<CS32)   | (0<<CS31)    | (1<<CS30);  // clock source = sys_clk / 1 (No prescaling)

    ETIMSK |= (1<<OCIE3A);				// compare A interrupt
}
//---------------------------------------------------------------------------------------------
ISR(TIMER3_COMPA_vect)
{
    // phase 0: just repeat same duration, but invert output.
    // phase 1: create new bit.

    if (!(PIND & (1<<DCC_PROG))) // phase 0: just repeat same duration, but invert output.
	{
        TCCR3A = (1<<COM3A1) | (1<<COM3A0)  // pin OC3A = "1" on compare match (DCC_PROG)
               | (1<<COM3B1) | (0<<COM3B0)  // pin OC3B = "0" on compare match (NDCC_PROG)
			   | (0<<FOC3A)  | (0<<FOC3B)
               | (0<<WGM31)  | (0<<WGM30);
    }
	else // phase 1: create new bit
	{
		//for testing impulses
		//TCCR3A = (1<<COM3A1) | (0<<COM3A0)	// pin OC3A = "0" on compare match (DCC_PROG)
	    //       | (1<<COM3B1) | (1<<COM3B0)	// pin OC3B = "1" on compare match (NDCC_PROG)
		//		 | (0<<FOC3A)  | (0<<FOC3B)
	    //       | (0<<WGM31)  | (0<<WGM30);
		//return;

		switch (doi.State)
		{
	        case Idle:
				if (Repeats == 0) // done; look in Queue
				{
					Message_t *pMsg = NULL;
					if (PROG_IS_ON)
					{
						pMsg = GetFromProgramQueue();
						SetProgPhase(IsServiceProgCommand(pMsg));
					}
					SetNextMessage(pMsg != NULL ? pMsg : &DCCIdle);
				}
				
				// NextMessage now is set (msg or DCCIdle), Repeats > 0
				Repeats--;
				doi.BytesRemained = doi.Size;
				doi.CurrentByteIdx = 0;
				doi.XORByte = 0;
				doi.BitsRemained = PROGRAM_PREAMBLE_LENGTH;
				doi.State = Preamble;
				break;
	        case Preamble:
	            SetBit(1);
	            doi.BitsRemained--;
	            if (doi.BitsRemained == 0) // preample finished; now send start bit
	                doi.State = StartBit;
	            break;
	        case StartBit:
	            SetBit(0);
	            if (doi.BytesRemained == 0) // all bytes are sent, now send XOR byte
	            {
	                doi.CurrentByte = doi.XORByte;
	                doi.BitsRemained = 8;
	                doi.State = XOR;
	            }
	            else // get next byte
	            {
	                doi.BytesRemained--;
	                doi.CurrentByte = doi.Dcc[doi.CurrentByteIdx++];
	                doi.XORByte ^= doi.CurrentByte;
	                doi.BitsRemained = 8;
	                doi.State = Byte;
	            }
	            break;
	        case Byte: // data byte
	            SetBit((doi.CurrentByte & 0x80 ?  1: 0)); // 0b10000000 - most left bit
	            doi.CurrentByte <<= 1; // bit sent, shift to next bit
	            doi.BitsRemained--;
	            if (doi.BitsRemained == 0)
	                doi.State = StartBit;
	            break;
	        case XOR: // error sum
	            SetBit((doi.CurrentByte & 0x80 ?  1: 0)); // 0b10000000 - most left bit
	            doi.CurrentByte <<= 1; // bit sent, shift to next bit
	            doi.BitsRemained--;
	            if (doi.BitsRemained == 0) // XOR is sent
	                doi.State = EndBit;
	            break;
			case EndBit:
	            SetBit(1);
				doi.State = Idle;
	            break;
			default:
				break;
		}
	}
}
//---------------------------------------------------------------------------------------------
