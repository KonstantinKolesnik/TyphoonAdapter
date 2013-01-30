#include "Hardware.h"
#include <avr/io.h>
#include <avr/iom162.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "USB.h"
#include "ShortCircuitDetector.h"
#include "OperationDCCOut.h"
#include "ProgramDCCOut.h"
#include "AckDetector.h"
//---------------------------------------------------------------------------------------------
void InitHardware()
{
	ACSR = 0b10000000;					// ACD bit = On: Analog comparator disabled
	
	// Port A
	PORTA	|= (0<<EN_MAIN)				// off
			|  (0<<LED_SHORT_MAIN)		// off
			|  (0<<EN_PROG)				// off
			|  (0<<LED_SHORT_PROG)		// off
			|  (0<<LED_MSG)				// off
			|  (1<<SHORT_MAIN)			// pull up
			|  (1<<SHORT_PROG)			// pull up
			|  (1<<ACK_DETECT)			// pull up
			;
	DDRA	|= (1<<EN_MAIN)				// out
			|  (1<<LED_SHORT_MAIN)		// out
			|  (1<<EN_PROG)				// out
			|  (1<<LED_SHORT_PROG)		// out
			|  (1<<LED_MSG)				// out
			|  (0<<SHORT_MAIN)			// in
			|  (0<<SHORT_PROG)			// in
			|  (0<<ACK_DETECT)			// in
			;
		  
	// Port B	  
	PORTB	|= (1<<RXD1)				// pull up
			|  (1<<TXD1)				// on
			|  (0<<NDCC_PROG);			// off
	DDRB	|= (0<<RXD1)    			// in
			|  (1<<TXD1)				// out
			|  (1<<NDCC_PROG); 			// out
		  
	// Port C
	PORTC	 = 0;						// all off
	DDRC	 = 255;						// all outs
	
	// Port D
	PORTD	|= (1<<RXD0)				// pull up
			|  (1<<TXD0)				// on
	        |  (0<<USB_DPLUS)			// pull down
			|  (0<<USB_DMINUS)			// pull down
			|  (0<<DCC_PROG)			// off
			|  (0<<DCC_MAIN)			// off
			;
	DDRD	|= (0<<RXD0)    			// in
			|  (1<<TXD0)				// out
	        |  (0<<USB_DPLUS)     		// in
			|  (0<<USB_DMINUS)     		// in
			|  (1<<DCC_PROG)   			// out
			|  (1<<DCC_MAIN)  			// out
			;
			
	// Port E
	PORTE	|= (0<<NDCC_MAIN);			// off
	DDRE	|= (1<<NDCC_MAIN); 			// out
}
void InitInterrupt()
{
    TIMSK |= (1<<OCIE1A)     // Timer/Counter1 Compare A Interrupt; reassigned in InitDCCOut()
          |  (0<<OCIE1B)     // Timer/Counter1 Compare B Interrupt
          |  (0<<TOIE1)      // Timer/Counter1 Overflow Interrupt
		  |  (0<<TICIE1)     // Timer/Counter1 Capture Interrupt
		  
	      |  (0<<OCIE2)      // Timer/Counter2 Compare Interrupt
          |  (0<<TOIE2)		 // Timer/Counter2 Overflow Interrupt
		  
		  |  (1<<OCIE0)      // Timer/Counter0 Compare Interrupt; reassigned in InitShortCircuitDetector()
		  |  (0<<TOIE0);     // Timer/Counter0 Overflow Interrupt
		  
    ETIMSK |= (1<<OCIE3A)    // Timer/Counter3 Compare A Interrupt; reassigned in InitDCCOut()
           |  (0<<OCIE3B)    // Timer/Counter3 Compare B Interrupt
           |  (0<<TOIE3)     // Timer/Counter3 Overflow Interrupt
		   |  (0<<TICIE3);   // Timer/Counter3 Capture Interrupt
		  
	sei();
}
//---------------------------------------------------------------------------------------------
int main()
{
	InitUSB();
	InitHardware();
	InitShortCircuitDetector();
	InitOperationDCCOut();
	InitProgramDCCOut();
	InitInterrupt();
	
	MAIN_TRACK_OFF;
	PROG_TRACK_OFF;
	
    while (1)
    {
		RunUSB();
		CheckAcknowledgement();
    }
}
//---------------------------------------------------------------------------------------------
