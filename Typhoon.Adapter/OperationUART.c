#include "Hardware.h"
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "OperationQueue.h"
#include "UART.h"
#include "OperationUART.h"
/*
//---------------------------------------------------------------------------------------------
#define my_UCSRA  UCSRA
	#define my_RXC    RXC
    #define my_TXC    TXC
    #define my_UDRE   UDRE
    #define my_FE     FE
    #define my_DOR    DOR
    #define my_UPE    UPE
    #define my_U2X    U2X
    #define my_MPCM    MPCM
#define my_UCSRB  UCSRB
	#define my_RXCIE  RXCIE
	#define my_TXCIE  TXCIE
    #define my_UDRIE  UDRIE
    #define my_RXEN   RXEN
    #define my_TXEN   TXEN
    #define my_UCSZ2  UCSZ2
	//#define my_RXB8   RXB8
	#define my_TXB8   TXB8
#define my_UCSRC  UCSRC
	//#define my_URSEL  URSEL // ?????
    #define my_UMSEL  UMSEL
    #define my_UPM1   UPM1 
    #define my_UPM0   UPM0
    #define my_USBS   USBS
    #define my_UCSZ1  UCSZ1 
    #define my_UCSZ0  UCSZ0
    #define my_UCPOL  UCPOL
	
#define my_UBRRL  UBRRL			// Baud rate register, LSB
#define my_UBRRH  UBRRH			// Baud rate register, MSB
#define my_UDR    UDR			// IO data register
//---------------------------------------------------------------------------------------------

#define MAX_RECEIVE_SIZE 7 											// pc message max bytes
static volatile unsigned char ReceiveBuffer[MAX_RECEIVE_SIZE];		// received pc message
static volatile unsigned char rxWriteIdx = 0;        				// current write or read index

#define MAX_SEND_SIZE 5
static volatile unsigned char SendBuffer[MAX_SEND_SIZE];
static volatile unsigned char txWriteIdx = 0;
static volatile unsigned char txReadIdx = 0;

Message_t msg;

//static void AddToSendQueue(const unsigned char c);
//static unsigned char* GetFromSendQueue();
static bool IsTxFull();
static bool IsTxEmpty();

//---------------------------------------------------------------------------------------------
void InitOperationUART(BaudRate baudRate)
{
	uint8_t sreg = SREG;
	uint8_t dummy;
	uint16_t ubrr; // baud rate value
	
    cli();

	my_UCSRB = 0;

	// calculations are done at mult 100 to avoid integer cast errors +50 is added
    
	switch (baudRate)
	{
		case BAUD_2400:
		    // ubrr = (uint16_t) ((uint32_t) F_CPU/(16*2400L) - 1);
            ubrr = (uint16_t) ((uint32_t)(F_CPU/(16*24L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC);
		    break;
		case BAUD_4800:
		    // ubrr = (uint16_t) ((uint32_t) F_CPU/(16*4800L) - 1);
            ubrr = (uint16_t) ((uint32_t) (F_CPU/(16*48L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC);
		    break;
		case BAUD_9600:
		    //   UBRRL = 103; // 9600bps @ 16.00MHz
			// ubrr = (uint16_t) ((uint32_t) F_CPU/(16*9600L) - 1);
            ubrr = (uint16_t) ((uint32_t) (F_CPU/(16*96L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC);
		    break;
	    default:
		case BAUD_19200:
		    //   UBRRL = 51; // 19200bps @ 16.00MHz
			//ubrr = (uint16_t) ((uint32_t) F_CPU/(16*19200L) - 1);
            ubrr = (uint16_t) ((uint32_t) (F_CPU/(16*192L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC);
		    break;
		case BAUD_38400:
		    //   UBRRL = 25; // 38400bps @ 16.00MHz
			// ubrr = (uint16_t) ((uint32_t) F_CPU/(16*38400L) - 1);
            ubrr = (uint16_t) ((uint32_t) (F_CPU/(16*384L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC) ;
		    break;
		case BAUD_57600:
		    // ubrr = (uint16_t) ((uint32_t) F_CPU/(8*57600L) - 1);
            ubrr = (uint16_t) ((uint32_t) (F_CPU/(8*576L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC) | (1 << my_U2X);  // High Speed Mode, nur div 8
		    break;
		case BAUD_115200:
		    // ubrr = (uint16_t) ((uint32_t) F_CPU/(8*115200L) - 1);
            ubrr = (uint16_t)((uint32_t)(F_CPU/(8*1152L) - 100L + 50L) / 100);
			my_UCSRA = (1 << my_RXC) | (1 << my_TXC) | (1 << my_U2X);  // High Speed Mode
		    break;
	}
    my_UBRRH = (uint8_t)(ubrr>>8);
    my_UBRRL = (uint8_t)ubrr;

	// enable Receiver, Transmitter, RX interrupt
    my_UCSRB = 0; // stop everything
    my_UCSRB = (1 << my_RXEN) | (1 << my_TXEN) | (1 << my_RXCIE);

    // Data mode 8N1, async
    my_UCSRC = (0 << my_UMSEL)						// 0 = asynchronous mode
             | (0 << my_UPM1) | (0 << my_UPM0)      // 00 = parity disabled
             | (0 << my_USBS)						// 1 = tx with 2 stop bits, 0 = tx with 1 stop bit
             | (1 << my_UCSZ1) | (1 << my_UCSZ0)	// 11 = 8 or 9 bits, depends on my_UCSZ2
             | (0 << my_UCPOL);						// clock polarity; 0 for async

    // Flush Receive-Buffer
    do
    {
        dummy = my_UDR;
    }
    while (my_UCSRA & (1 << my_RXC));
	
    my_UCSRA |= (1 << my_RXC);
    my_UCSRA |= (1 << my_TXC);
    //my_UCSRA |= (0 << my_MPCM); // multiprocessor communication mode disabled
	
    dummy = my_UDR;
	//dummy = my_UCSRA;
	
    SREG = sreg;
}

inline static void ParseCommand()
{
	LED_RS232_ON;
	switch (ReceiveBuffer[0])
	{
		case 'D': // DCC command
			msg.Repeats = ReceiveBuffer[1];
			msg.Size = ReceiveBuffer[2];
			memcpy(msg.Dcc, &ReceiveBuffer[3], msg.Size);
			if (PROG_IS_ON)
				AddToProgramQueue(&msg);
			else if (MAIN_IS_ON)
				AddToOperationQueue(&msg);
			break;
		case 'S': // station command
			switch (ReceiveBuffer[1])
			{
				case '0': MAIN_TRACK_ON; break;
				case '1': MAIN_TRACK_OFF; break;
				case '2': PROG_TRACK_ON; break;
				case '3': PROG_TRACK_OFF; break;
				case '4': DCCOutEnableRailcom(); break;
				case '5': DCCOutDisableRailcom(); break;
			}				
			break;
		default: break;
	}
	LED_RS232_OFF;
}
ISR(USART_RX_vect) // byte received interrupt
{
	if (my_UCSRA & (1<< my_FE)) // Frame Error
	{
	}
	else
	{
        if (my_UCSRA & (1<< my_DOR)) // DATA Overrun -> Fatal!!!
		{
        }
        if (my_UCSRA & (1<< my_UPE)) // Parity error
		{
        }
		
       	unsigned char c = my_UDR; // received byte
		if (c == '*') // '*' - is end of command
		{
			ReceiveBuffer[rxWriteIdx] = 0;
			rxWriteIdx = 0;
			ParseCommand(); // ReceiveBuffer is w/o '*' at the end
		}
		else
		{
			ReceiveBuffer[rxWriteIdx] = c;
			rxWriteIdx++;
			if (rxWriteIdx >= MAX_RECEIVE_SIZE)
				rxWriteIdx = 0;
		}
	}	   
}

ISR(USART_UDRE_vect) // Data Register Empty interrupt
{
    if (!IsTxEmpty())
    {
        my_UCSRA |= (1 << my_TXC);		// writing 1 clears any existing tx complete flag
        my_UDR = SendBuffer[txReadIdx];
        txReadIdx++;
        if (txReadIdx == MAX_SEND_SIZE)
			txReadIdx = 0;
    }
    else
        my_UCSRB &= ~(1 << my_UDRIE);	// disable further TxINT
}

void USARTWriteChar(const unsigned char c)
{
	SendBuffer[txWriteIdx] = c;
	txWriteIdx++;
    if (txWriteIdx == MAX_SEND_SIZE)
		txWriteIdx = 0;
		
    my_UCSRB |= (1 << my_UDRIE);   // enable TxINT
}
void OperationUARTWriteString(char* s)
{
    LED_RS232_ON;
	while (*s)
	{
		USARTWriteChar(*s);
		//AddToSendQueue(s);
		s++;
	}		
	LED_RS232_OFF;
}






//static void AddToSendQueue(unsigned char* c)
//{
	//if (!IsTxFull())
	//{
		//memcpy(&SendBuffer[txWriteIdx], c, sizeof(unsigned char));
		//txWriteIdx++;
		//txWriteIdx %= MAX_SEND_SIZE;
	//}
//}
//static unsigned char* GetFromSendQueue()
//{
    //if (!IsTxEmpty())
    //{
		//uint8_t idx = txReadIdx;
        //txReadIdx++;
        //txReadIdx %= MAX_SEND_SIZE;
		//
		//return &SendBuffer[idx];
    //}
	//else
		//return NULL;
//}
static bool IsTxFull()
{
    return (((txWriteIdx + 1) % MAX_SEND_SIZE) == txReadIdx);
}
static bool IsTxEmpty()
{
    return (txReadIdx == txWriteIdx);
}


*/