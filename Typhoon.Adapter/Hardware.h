#ifndef HARDWARE_H_
#define HARDWARE_H_
//---------------------------------------------------------------------------------------------
// 1. Processor Definitions
//---------------------------------------------------------------------------------------------
// ATMega162: 16 KBytes FLASH, 1024 Byte SRAM, 512 Byte EEPROM
// Fuses: High = 0x9F, Low = 0xDE, Extend = 0xFF
#define F_CPU 12000000UL
//---------------------------------------------------------------------------------------------
// 2. Ports Definition
//---------------------------------------------------------------------------------------------
// Port B
//								0	//PB0 (OC0/T0)				pin 1
//								1	//PB1 (OC2/T1)				pin 2
#define RXD1					2	//PB2 (RXD1/AIN0)			pin 3
#define TXD1					3	//PB3 (TXD1/AIN1)			pin 4
#define NDCC_PROG				4	//PB4 (SS/OC3B)				pin 5
//								5	//PB5 (MOSI)				pin 6
//								6	//PB6 (MISO)				pin 7
//								7	//PB7 (SCK)					pin 8
//---------------------------------------------------------------------------------------------
// Port D
#define RXD0					0	//PD0 (RXD0)				pin 10
#define TXD0					1	//PD1 (TXD0)				pin 11
#define USB_DPLUS				2	//PD2 (INT0/XCK1)			pin 12
#define USB_DMINUS				3	//PD3 (INT1/ICP3)			pin 13
#define DCC_PROG				4	//PD4 (TOSC1/XCK0/OC3A)		pin 14
#define DCC_MAIN				5	//PD5 (OC1A/TOSC2) 			pin 15
//								6	//PD6 (WR) 					pin 16
//								7	//PD7 (RD) 					pin 17
//---------------------------------------------------------------------------------------------
// Port A
#define EN_MAIN					0	//PA0 (AD0/PCINT0)			pin 39
#define LED_SHORT_MAIN			1	//PA1 (AD1/PCINT1)			pin 38
#define EN_PROG					2	//PA2 (AD2/PCINT2)			pin 37
#define LED_SHORT_PROG			3	//PA3 (AD3/PCINT3)			pin 36
#define LED_MSG					4	//PA4 (AD4/PCINT4)			pin 35
#define SHORT_MAIN				5	//PA5 (AD5/PCINT5)			pin 34
#define SHORT_PROG				6	//PA6 (AD6/PCINT6)			pin 33
#define ACK_DETECT				7	//PA7 (AD7/PCINT7)			pin 32
//---------------------------------------------------------------------------------------------
// Port C
//								0	//PC0 (A8/PCINT8)			pin 21
//								1	//PC1 (A9/PCINT9)			pin 22
//								2	//PC2 (A10/PCINT10)			pin 23
//								3	//PC3 (A11/PCINT11)			pin 24
//JTAG							4	//PC4 (A12/TCK/PCINT12)		pin 25
//JTAG							5	//PC5 (A13/TMS/PCINT13)		pin 26
//JTAG							6	//PC6 (A14/TDO/PCINT14)		pin 27
//JTAG							7	//PC7 (A15/TDI/PCINT15)		pin 28
//---------------------------------------------------------------------------------------------
// Port E
//								0	//PE0 (ICP1/INT2)			pin 31
//								1	//PE1 (ALE)					pin 30
#define NDCC_MAIN				2	//PE2 (OC1B)				pin 29
//---------------------------------------------------------------------------------------------
// 3. LED-Control and IO-Macros
//---------------------------------------------------------------------------------------------
#define MAIN_TRACK_ON    		PORTA |= (1<<EN_MAIN)
#define MAIN_TRACK_OFF   		PORTA &= ~(1<<EN_MAIN)
#define MAIN_IS_ON 				(PINA & (1<<EN_MAIN))
#define MAIN_SHORT_DETECTED		(!(PINA & (1<<SHORT_MAIN)))
#define MAIN_SHORT_BLOCKED		(PINA & (1<<LED_SHORT_MAIN))

#define PROG_TRACK_ON    		PORTA |= (1<<EN_PROG)
#define PROG_TRACK_OFF   		PORTA &= ~(1<<EN_PROG)
#define PROG_IS_ON		 		(PINA & (1<<EN_PROG))
#define PROG_SHORT_DETECTED		(!(PINA & (1<<SHORT_PROG)))
#define PROG_SHORT_BLOCKED		(PINA & (1<<LED_SHORT_PROG))

#define ACK_DETECTED	  		(!(PINA & (1<<ACK_DETECT)))

#define LED_MSG_ON	     		PORTA |= (1<<LED_MSG)
#define LED_MSG_OFF	    		PORTA &= ~(1<<LED_MSG)
//#define LED_MSG_IS_ON			(PINA & (1<<LED_MSG))

#define LED_SHORT_MAIN_ON     	PORTA |= (1<<LED_SHORT_MAIN)
#define LED_SHORT_MAIN_OFF    	PORTA &= ~(1<<LED_SHORT_MAIN)

#define LED_SHORT_PROG_ON     	PORTA |= (1<<LED_SHORT_PROG)
#define LED_SHORT_PROG_OFF    	PORTA &= ~(1<<LED_SHORT_PROG)
//---------------------------------------------------------------------------------------------
#endif