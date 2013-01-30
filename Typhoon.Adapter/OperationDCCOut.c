#include "Hardware.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "DCCMessage.h"
#include "OperationDCCOut.h"
#include "OperationQueue.h"
//---------------------------------------------------------------------------------------------
static uint8_t isRailcomEnabled = 0;
static Message_t DCCIdle;
static DCCOutputInfo doi;

/*
#if (TURNOUT_FEEDBACK_ENABLED == 1)
volatile unsigned int feedbacked_accessory; // this is the current turnout under query; this includes the coil address!
volatile unsigned char feedback_required;   // 1: make a query at end of next preamble
volatile unsigned char feedback_ready;      // MSB: if 1: the query is done (communication flag)
                                            // LSB: if 1: there was no feedback
                                            //            == position error
*/
//---------------------------------------------------------------------------------------------
static void SetNextMessage(Message_t *msg);
static inline void SetBit(uint8_t bit) __attribute__((always_inline));
static inline void SetBit_no_B(uint8_t bit) __attribute__((always_inline));
//---------------------------------------------------------------------------------------------
// DCC generator Interface
//---------------------------------------------------------------------------------------------
static void SetNextMessage(Message_t *pMsg)
{
	memcpy(doi.Dcc, pMsg->Dcc, pMsg->Size);
	doi.Size = pMsg->Size;
	doi.Repeats = pMsg->Repeats;
	//doi.Type = pMsg->Type;      	// remember type in case feedback is required
}
static void SetBit(uint8_t bit)
{
	TCCR1A = (1<<COM1A1) | (0<<COM1A0)  // pin OC1A = "0" on compare match (DCC_MAIN)
           | (1<<COM1B1) | (1<<COM1B0)  // pin OC1B = "1" on compare match (NDCC)
		   | (0<<FOC1A)  | (0<<FOC1B)
           | (0<<WGM11)  | (0<<WGM10);

	OCR1A = OCR1B = F_CPU * (bit == 0 ? PERIOD_0 : PERIOD_1) / 2 / 1000000L; // 1392/696 ticks
}
static void SetBit_no_B(uint8_t bit) // this is the code for cutout - lead_in
{
    TCCR1A = (1<<COM1A1) | (0<<COM1A0)  // pin OC1A = "0" on compare match (DCC_MAIN)
           | (1<<COM1B1) | (1<<COM1B0)  // pin OC1B = "1" on compare match (NDCC)
		   | (0<<FOC1A)  | (0<<FOC1B)
           | (0<<WGM11)  | (0<<WGM10);

	if (bit == 0) //  0 - make a long pwm pulse
    {
        OCR1A = F_CPU * PERIOD_0 / 2 / 1000000L;     //1856 (for 16MHz)
        OCR1B = F_CPU * PERIOD_0 / 2 / 1000000L * 4; // extended (cutout starts after OCR1A)
    }
    else //  1 - make a short pwm puls
    {
        // OCR1A = F_CPU * PERIOD_1  / 2 / 1000000L ;            //928
        OCR1A = F_CPU * CUTOUT_GAP / 1000000L ;            //928
        OCR1B = F_CPU * PERIOD_1 / 2 / 1000000L * 8;          // extended (cutout starts after OCR1A)
    }
}
void InitOperationDCCOut()
{
	DCCIdle.Size = 2;
	DCCIdle.Repeats = 1;
    DCCIdle.Dcc[0] = 255;
    DCCIdle.Dcc[1] = 0;

    doi.State = Idle;
	OperationDCCOutDisableRailcom();

    //#if (TURNOUT_FEEDBACK_ENABLED == 1)
    //	feedback_ready = 0;
	//	feedback_required = 0;
    //#endif

	// set Timer/Counter1
    TCNT1 = 0; // no prescaler
    
	TCCR1A = (1<<COM1A1) | (0<<COM1A0)  // pin OC1A = "0" on compare match (DCC_MAIN)
           | (1<<COM1B1) | (1<<COM1B0)  // pin OC1B = "1" on compare match (NDCC_MAIN)
		   | (0<<FOC1A)  | (0<<FOC1B)   // reserved in PWM, set to zero
           | (0<<WGM11)  | (0<<WGM10);  // (+ WGM12, WGM13) CTC mode, TOP = OCR1A
    
	TCCR1B = (0<<ICNC1)  | (0<<ICES1)   // Input Capture Noise Canceler off; Input Capture Edge Select off
           | (0<<WGM13)  | (1<<WGM12)	// (+ WGM10, WGM10) CTC mode, TOP = OCR1A
           | (0<<CS12)   | (0<<CS11)   | (1<<CS10);  // clock source = sys_clk / 1 (no prescaling)

    TIMSK |= (1<<OCIE1A);				// compare A interrupt enabled
}
//---------------------------------------------------------------------------------------------
// RailCom Interface
//---------------------------------------------------------------------------------------------
void OperationDCCOutEnableRailcom()
{
    isRailcomEnabled = 1;
}
void OperationDCCOutDisableRailcom()
{
    isRailcomEnabled = 0;
}
uint8_t OperationDCCOutQueryRailcom()
{
    return isRailcomEnabled;
}
//---------------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
    // phase 0: just repeat same duration, but invert output.
    // phase 1: create new bit.

    if (!(PIND & (1<<DCC_MAIN))) // phase 0: just repeat same duration, but invert output.
	{
		if (doi.State == Cutout2 && isRailcomEnabled)
        {
			TCCR1A = (1<<COM1A1) | (1<<COM1A0)  // pin OC1A = "1" on compare match (DCC_MAIN)
                   | (1<<COM1B1) | (1<<COM1B0)  // pin OC1B = "1" on compare match (NDCC_MAIN)
				   | (0<<FOC1A)  | (0<<FOC1B)
                   | (0<<WGM11)  | (0<<WGM10);
				   
    		OCR1A = (F_CPU / 1000000L * PERIOD_1 * 4)     - (F_CPU / 1000000L * CUTOUT_GAP);	// create extended timing: 4   * PERIOD_1 for DCC  - GAP
    		OCR1B = (F_CPU / 1000000L * PERIOD_1 * 9 / 2) - (F_CPU / 1000000L * CUTOUT_GAP);	// create extended timing: 4.5 * PERIOD_1 for NDCC - GAP
    	}
        else
        {
			// just invert output
            TCCR1A = (1<<COM1A1) | (1<<COM1A0)  // pin OC1A = "1" on compare match (DCC_MAIN)
                   | (1<<COM1B1) | (0<<COM1B0)  // pin OC1B = "0" on compare match (NDCC_MAIN)
				   | (0<<FOC1A)  | (0<<FOC1B)
                   | (0<<WGM11)  | (0<<WGM10);
    	}
    }
	else // phase 1: create new bit
	{
		//for testing impulses
		//TCCR1A = (1<<COM1A1) | (0<<COM1A0)	// pin OC1A = "0" on compare match (DCC_MAIN)
	    //       | (1<<COM1B1) | (1<<COM1B0)	// pin OC1B = "1" on compare match (NDCC_MAIN)
		//		 | (0<<FOC1A)  | (0<<FOC1B)
	    //       | (0<<WGM11)  | (0<<WGM10);
		//return;

		switch (doi.State)
		{
	        case Idle:
				if (doi.Repeats == 0) // done; look in Queue for the next command
				{
					Message_t *pMsg = NULL;
					if (MAIN_IS_ON)
						pMsg = GetFromOperationQueue();
					SetNextMessage(pMsg != NULL ? pMsg : &DCCIdle);
				}
				
				// now NextMessage is set (msg or DCCIdle), Repeats > 0
				doi.Repeats--;
				doi.BytesRemained = doi.Size;
				doi.CurrentByteIdx = 0;
				doi.XORByte = 0;
				doi.BitsRemained = OPERATION_PREAMBLE_LENGTH;
				doi.State = Preamble;
				break;
	        case Preamble:
	            SetBit(1);
	            doi.BitsRemained--;
	            if (doi.BitsRemained == 0) // preample finished; now send start bit
	            {
	                doi.State = StartBit;
	                /*
					#if (TURNOUT_FEEDBACK_ENABLED == 1)
	                if (feedback_required)
	                {
	                    if (EXT_STOP_PRESSED) 
	                        // feedback received -> yes, turnout has this position
	                        feedback_ready = (1 << FB_READY) | (1 << FB_OKAY);
	                    else
	                        // no feedback: 
	                        feedback_ready = (1 << FB_READY);
	                    feedback_required = 0;
	                }
	                #endif
					*/
	            }
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
	            {
	                doi.State = EndBit;

	                /*
					#if (TURNOUT_FEEDBACK_ENABLED == 1)
	                if (doi.Type == is_feedback)
	                {
	                    // message1 message0 -> ...||......|
	                    // -aaa-ccc --AAAAAA => aaaAAAAAAccc
	                    feedbacked_accessory = ((doi.dcc[0] & 0b00111111) << 3)   // addr low
	                                       | ((~doi.dcc[1] & 0b01110000) << 5)    // addr high
	                                       | (doi.dcc[1] & 0b00000111);           // output
	                    feedback_ready = 0;
	                    feedback_required = 1;
	                }
	                #endif // feedback
					*/
	            }
	            break;
			case EndBit:
	            SetBit(1);
	            if (isRailcomEnabled)
					doi.State = Cutout1;
	            else
					doi.State = Idle;
	            break;
	        case Cutout1:
	            if (isRailcomEnabled)
					SetBit_no_B(1);     // first 1 after message gets extended
	            else
					SetBit(1);
	            doi.State = Cutout2;
	            break;
	        case Cutout2:
	            SetBit(1);
	            doi.State = Idle;
	            break;
			default:
				break;
		}
	}
}
//---------------------------------------------------------------------------------------------
