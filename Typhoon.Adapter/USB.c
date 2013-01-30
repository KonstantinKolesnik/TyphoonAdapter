#include "Hardware.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "USBDriver/usbdrv.h"
#include "DCCMessage.h"
#include "OperationQueue.h"
#include "OperationDCCOut.h"
#include "ProgramQueue.h"
#include "AckDetector.h"
//---------------------------------------------------------------------------------------------
typedef struct
{
   uchar Type;
   uchar Data[10];
} Packet; // data transfer; 10 data bytes + 1 type byte + 1 usb byte is for packet id (=0)
//---------------------------------------------------------------------------------------------
static Packet packet;
static Message_t msg;
static uchar readIdx;
static uchar bytesRemaining;
PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = 
{
    0x05, 0x01,			                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x03,                             // USAGE (Vendor Usage 1)
    0xA1, 0x01,                             // COLLECTION (Application)
	
	0x15, 0x00,     						// LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00, 						// LOGICAL_MAXIMUM (255)
	0x75, 0x08,     						// REPORT_SIZE (8 bits)
	0x95, sizeof(Packet),     				// REPORT_COUNT (bytes)
	
	// The Input report
	//0x85, 0x77,								// REPORT_ID (0x77)
	0x09, 0x03,     						// USAGE (Vendor Usage 1)
	0x81, 0x02,     						// INPUT (Data, Variable, Absolute)

	// The Output report
	//0x85, 0x78,								// REPORT_ID (0x78)
	0x09, 0x03,     						// USAGE (Vendor Usage 1)
	0x91, 0x02,      						// OUTPUT (Data, Variable, Absolute)

	// The Feature report
	//0x09, 0x01,     						// USAGE ID - vendor defined
	//0x15, 0x00,     						// LOGICAL_MINIMUM (0)
	//0x26, 0xFF, 0x00, 					// LOGICAL_MAXIMUM (255)
	//0x75, 0x08,     						// REPORT_SIZE (8 bits)
	//0x95, 0x40,     						// REPORT_COUNT (64 fields)
	//0xB1, 0x02,      						// Feature (Data, Variable, Absolute)

    0xC0                                    // END_COLLECTION
};
//---------------------------------------------------------------------------------------------
void PrepareAnswer()
{
	LED_MSG_ON;
	packet.Type = 'S';
	packet.Data[0] = MAIN_IS_ON;
	packet.Data[1] = PROG_IS_ON;
	packet.Data[2] = MAIN_SHORT_BLOCKED;
	packet.Data[3] = PROG_SHORT_BLOCKED;
	packet.Data[4] = OperationDCCOutQueryRailcom();
	packet.Data[5] = QueryAckFlag();
	LED_MSG_OFF;
}
// вызываетс€, когда хост запрашивает порцию данных от устройства; см. usbdrv.h
uchar usbFunctionRead(uchar *data, uchar len)
{
    if (len > bytesRemaining)
        len = bytesRemaining;

    if (!readIdx) // Ќи один кусок данных еще не прочитан. «аполним структуру дл€ передачи.
		PrepareAnswer();

    uchar *buffer = (uchar*)&packet;
    uchar j;
    for (j = 0; j < len; j++)
        data[j] = buffer[j + readIdx];

    readIdx += len;
    bytesRemaining -= len;
	
    return len;
}
//---------------------------------------------------------------------------------------------
static inline void ParseCommand()
{
	LED_MSG_ON;
	switch (packet.Type)
	{
		case 'D': // DCC command
			msg.Repeats = packet.Data[1];
			msg.Size = packet.Data[2];
			memcpy(msg.Dcc, &packet.Data[3], msg.Size);
			switch (packet.Data[0])
			{
				case 'O': if (MAIN_IS_ON) AddToOperationQueue(&msg); break;
				case 'P': /*if (PROG_IS_ON)*/ AddToProgramQueue(&msg); break;
				default: break;
			}
			break;
		case 'S': // station command
			switch (packet.Data[0])
			{
				case '0': MAIN_TRACK_ON; break;
				case '1': MAIN_TRACK_OFF; break;
				case '2': PROG_TRACK_ON; break;
				case '3': PROG_TRACK_OFF; break;
				case '4': OperationDCCOutEnableRailcom(); break;
				case '5': OperationDCCOutDisableRailcom(); break;
				case '6': ClearProgramQueue(); ResetAckFlag(); break;
				default: break;
			}				
			break;
		default: break;
	}
	LED_MSG_OFF;
}
// вызываетс€, когда хост отправл€ет порцию данных к устройству; см. usbdrv.h
uchar usbFunctionWrite(uchar *data, uchar len)
{
    if (bytesRemaining == 0) // конец передачи
        return 1;
		
    if (len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&packet;
    uchar j;
    for (j = 0; j < len; j++)
        buffer[j + readIdx] = data[j];

    readIdx += len;
    bytesRemaining -= len;
	
    if (bytesRemaining == 0) // ¬се данные получены
		ParseCommand();			

    return bytesRemaining == 0; // false означает, что есть еще данные
}
//---------------------------------------------------------------------------------------------
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (void*)data;
    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) // HID устройство
	{   
        if (rq->bRequest == USBRQ_HID_GET_REPORT)
		{
			/* wValue: ReportType (highbyte), ReportID (lowbyte) */
            // у нас только одна разновидность репорта, можем игнорировать report-ID
            bytesRemaining = sizeof(Packet);
            readIdx = 0;
            return USB_NO_MSG;  // используем usbFunctionRead() дл€ отправки данных хосту
        }
		else if (rq->bRequest == USBRQ_HID_SET_REPORT)
		{
            // у нас только одна разновидность репорта, можем игнорировать report-ID
            bytesRemaining = sizeof(Packet);
            readIdx = 0;
            return USB_NO_MSG;  // используем usbFunctionWrite() дл€ получени€ данных от хоста
        }
    }
	else
	{
        /* остальные запросы мы просто игнорируем */
    }
    return 0;
}
//---------------------------------------------------------------------------------------------
void InitUSB()
{
	wdt_enable(WDTO_1S);
	usbInit();
    usbDeviceDisconnect();  // принудительно отключаемс€ от хоста, так делать можно только при выключенных прерывани€х!
    unsigned char i = 0;
    while(--i)             // пауза > 250 ms
	{
		wdt_reset();
        _delay_ms(1);
	}		
    usbDeviceConnect();
}
void RunUSB()
{
	wdt_reset();
	usbPoll();          // эту функцию надо регул€рно вызывать с главного цикла, максимальна€ задержка между вызовами - 50 ms
}
//---------------------------------------------------------------------------------------------
