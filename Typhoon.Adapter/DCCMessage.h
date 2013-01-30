#ifndef DCCMESSAGE_H_
#define DCCMESSAGE_H_
//---------------------------------------------------------------------------------------------
#include "Hardware.h"
//---------------------------------------------------------------------------------------------
#define OPERATION_PREAMBLE_LENGTH	15  	// min 14;
#define PROGRAM_PREAMBLE_LENGTH		22  	// min 20;
#define MAX_DCC_SIZE				5		// (up to 2) for address + (up to 3) for data

#define PERIOD_1					116L	// 116us for DCC 1 pulse - do not change
#define PERIOD_0					232L	// 232us for DCC 0 pulse - do not change
#define CUTOUT_GAP					38L		// 38us gap after last bit of XOR

#if ((F_CPU / 1024 * PERIOD_0) > 4177000)
#warning: Overflow in calculation of constant
// if this warning is issued, please split the "/1000000L" into two parts
#endif

/*
typedef enum {is_void,      // message with no special handling (like functions)
              is_stop,      // broadcast
              is_loco,      // standard dcc speed command
              is_acc,       // accessory command
              is_feedback,  // accessory command with feedback
              is_prog,      // service mode - longer preambles
              is_prog_ack}  MessageType;
*/

typedef struct 
{
	uint8_t			Repeats;
    uint8_t			Size;
    uint8_t			Dcc[MAX_DCC_SIZE];
    //MessageType   Type;
} Message_t;

typedef enum
{
     Idle,
     Preamble,
     StartBit,
     Byte,
     XOR,
     EndBit,
     Cutout1,
     Cutout2
} DCCOutputState;

typedef struct
{
    DCCOutputState	State;
	
    uint8_t			Dcc[MAX_DCC_SIZE];		// current message data bytes
	uint8_t			Size;				    // current message bytes count
	uint8_t			Repeats;			    // current message repeat count
    //MessageType	Type;					// current message type (for feedback)
	
	uint8_t			BytesRemained;			// number of bytes remained (decremented)
    uint8_t			BitsRemained;			// number of bits remained (decremented)
    
    uint8_t			CurrentByteIdx;			// current processing byte index
	uint8_t			CurrentByte;			// current processing byte value
    uint8_t			XORByte;				// XOR byte value
} DCCOutputInfo;
//---------------------------------------------------------------------------------------------
#endif