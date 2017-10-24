/*
 * uart_fifo.c
 *
 * Created: 10/22/2017 12:51:44 AM
 *  Author: Gaal Alexandru
 */ 

#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>

#ifndef RX_BUFFER_SIZE
	#define UART_RX_BUFFER_SIZE 256
#endif

#ifndef TX_BUFFER_SIZE
	#define UART_TX_BUFFER_SIZE 256
#endif

volatile uint8_t g_pcUARTRxBuffer[UART_RX_BUFFER_SIZE];  //Incoming message buffer
volatile uint8_t g_ui8UARTRxWriteIndex = 0;
volatile uint8_t g_ui8UARTRxReadIndex = 0;

volatile uint8_t g_pcUARTTxBuffer[UART_TX_BUFFER_SIZE];  //Outgoing message buffer
volatile uint8_t g_ui8UARTTxWriteIndex = 0;
volatile uint8_t g_ui8UARTTxReadIndex = 0;

//*****************************************************************************
//
// Macros to determine number of free and used bytes in the transmit buffer.
//
//*****************************************************************************
#define TX_BUFFER_USED          (GetBufferCount(&g_ui8UARTTxReadIndex,  \
                                                &g_ui8UARTTxWriteIndex, \
                                                UART_TX_BUFFER_SIZE))
#define TX_BUFFER_FREE          (UART_TX_BUFFER_SIZE - TX_BUFFER_USED)
#define TX_BUFFER_EMPTY         (IsBufferEmpty(&g_ui8UARTTxReadIndex,   \
                                               &g_ui8UARTTxWriteIndex))
#define TX_BUFFER_FULL          (IsBufferFull(&g_ui8UARTTxReadIndex,  \
                                              &g_ui8UARTTxWriteIndex, \
                                              UART_TX_BUFFER_SIZE))
#define ADVANCE_TX_BUFFER_INDEX(Index) \
                                (Index) = ((Index) + 1) % UART_TX_BUFFER_SIZE

//*****************************************************************************
//
// Macros to determine number of free and used bytes in the receive buffer.
//
//*****************************************************************************
#define RX_BUFFER_USED          (GetBufferCount(&g_ui8UARTRxReadIndex,  \
                                                &g_ui8UARTRxWriteIndex, \
                                                UART_RX_BUFFER_SIZE))
#define RX_BUFFER_FREE          (UART_RX_BUFFER_SIZE - RX_BUFFER_USED)
#define RX_BUFFER_EMPTY         (IsBufferEmpty(&g_ui8UARTRxReadIndex,   \
                                               &g_ui8UARTRxWriteIndex))
#define RX_BUFFER_FULL          (IsBufferFull(&g_ui8UARTRxReadIndex,  \
                                              &g_ui8UARTRxWriteIndex, \
                                              UART_RX_BUFFER_SIZE))
#define ADVANCE_RX_BUFFER_INDEX(Index)	(Index) = ((Index) + 1) % UART_RX_BUFFER_SIZE

//*****************************************************************************
//
//! Determines whether the ring buffer whose pointers and size are provided
//! is full or not.
//!
//! \param pui8Read points to the read index for the buffer.
//! \param pui8Write points to the write index for the buffer.
//! \param ui8Size is the size of the buffer in bytes.
//!
//! This function is used to determine whether or not a given ring buffer is
//! full.  The structure of the code is specifically to ensure that we do not
//! see warnings from the compiler related to the order of volatile accesses
//! being undefined.
//!
//! \return Returns \b true if the buffer is full or \b false otherwise.
//
//*****************************************************************************
static bool IsBufferFull(volatile uint8_t *pui8Read, volatile uint8_t *pui8Write, uint8_t ui8Size)
{
	uint8_t ui8Write;
	uint8_t ui8Read;

	ui8Write = *pui8Write;
	ui8Read = *pui8Read;

	return((((ui8Write + 1) % ui8Size) == ui8Read) ? true : false);
}

//*****************************************************************************
//
//! Determines whether the ring buffer whose pointers and size are provided
//! is empty or not.
//!
//! \param pui8Read points to the read index for the buffer.
//! \param pui8Write points to the write index for the buffer.
//!
//! This function is used to determine whether or not a given ring buffer is
//! empty.  The structure of the code is specifically to ensure that we do not
//! see warnings from the compiler related to the order of volatile accesses
//! being undefined.
//!
//! \return Returns \b true if the buffer is empty or \b false otherwise.
//
//*****************************************************************************
static bool IsBufferEmpty(volatile uint8_t *pui8Read, volatile uint8_t *pui8Write)
{
	uint8_t ui8Write;
	uint8_t ui8Read;

	ui8Write = *pui8Write;
	ui8Read = *pui8Read;

	return((ui8Write == ui8Read) ? true : false);
}

//*****************************************************************************
//
//! Determines the number of bytes of data contained in a ring buffer.
//!
//! \param pui8Read points to the read index for the buffer.
//! \param pui8Write points to the write index for the buffer.
//! \param ui8Size is the size of the buffer in bytes.
//!
//! This function is used to determine how many bytes of data a given ring
//! buffer currently contains.  The structure of the code is specifically to
//! ensure that we do not see warnings from the compiler related to the order
//! of volatile accesses being undefined.
//!
//! \return Returns the number of bytes of data currently in the buffer.
//
//*****************************************************************************
static uint8_t GetBufferCount(volatile uint8_t *pui8Read, volatile uint8_t *pui8Write, uint8_t ui8Size)
{
    uint8_t ui8Write;
    uint8_t ui8Read;

    ui8Write = *pui8Write;
    ui8Read =  *pui8Read;

    return((ui8Write >= ui8Read) ? (ui8Write - ui8Read) : (ui8Size - (ui8Read - ui8Write)));
}

//*****************************************************************************
//
//! Flushes the receive buffer.
//!
//! This function, available only when the module is built to operate in
//! buffered mode using \b UART_BUFFERED, may be used to discard any data
//! received from the UART but not yet read using UARTgets().
//!
//! \return None.
//
//*****************************************************************************
void UARTFlushRx(void)
{
	cli();//Temporarily Disable interrupts
    // Flush the receive buffer.
    g_ui8UARTRxReadIndex = 0;
    g_ui8UARTRxWriteIndex = 0;
	sei();  // Restore Global Interrupt Flag
}

//*****************************************************************************
//
//! Flushes the transmit buffer.
//!
//! \param bDiscard indicates whether any remaining data in the buffer should
//! be discarded (\b true) or transmitted (\b false).
//!
//! This function, available only when the module is built to operate in
//! buffered mode using \b UART_BUFFERED, may be used to flush the transmit
//! buffer, either discarding or transmitting any data received via calls to
//! UARTprintf() that is waiting to be transmitted.  On return, the transmit
//! buffer will be empty.
//!
//! \return None.
//
//*****************************************************************************
void UARTFlushTx(bool bDiscard)
{
    cli();//Temporarily Disable interrupts
    if(bDiscard) // Should the remaining data be discarded or transmitted?
    {
        // Flush the transmit buffer.
        g_ui8UARTTxReadIndex = 0;
        g_ui8UARTTxWriteIndex = 0;
	}
	else
	{  
		while(!TX_BUFFER_EMPTY) // Wait for all remaining data to be transmitted before returning.
		{
			// AleGaa if there is anything on TX buffer try to put it in TX Fifo
			// AleGaa TODO UARTPrimeTransmit(g_ui8Base);
		}
	}
	sei();  // Restore Global Interrupt Flag
}

//*****************************************************************************
//
//! Looks ahead in the receive buffer for a particular character.
//!
//! \param ucChar is the character that is to be searched for.
//!
//! This function, available only when the module is built to operate in
//! buffered mode using \b UART_BUFFERED, may be used to look ahead in the
//! receive buffer for a particular character and report its position if found.
//! It is typically used to determine whether a complete line of user input is
//! available, in which case ucChar should be set to CR ('\\r') which is used
//! as the line end marker in the receive buffer.
//!
//! \return Returns -1 to indicate that the requested character does not exist
//! in the receive buffer.  Returns a non-negative number if the character was
//! found in which case the value represents the position of the first instance
//! of \e ucChar relative to the receive buffer read pointer.
//
//*****************************************************************************
int16_t UARTPeek(uint8_t ucChar)
{
    uint8_t ui8Count;
    uint8_t ui8Avail;
    uint8_t ui8TempReadIndex;

    // How many characters are there in the receive buffer?
    ui8Avail = RX_BUFFER_USED;
    ui8TempReadIndex = g_ui8UARTRxReadIndex;

    // Check all the unread characters looking for the one passed.
    for(ui8Count = 0; ui8Count < ui8Avail; ui8Count++)
    {
        if(g_pcUARTRxBuffer[ui8TempReadIndex] == ucChar)
        {
            return( (uint8_t)ui8Count); // We found it so return the index
        }
        else
        {
			// This one didn't match so move on to the next character.
            ADVANCE_RX_BUFFER_INDEX(ui8TempReadIndex); 
        }
    }
    // If we drop out of the loop, we didn't find the character in the receive buffer.
    return(-1);
}

//*****************************************************************************
//
//! Returns the number of bytes available in the receive buffer.
//!
//! This function, available only when the module is built to operate in
//! buffered mode using \b UART_BUFFERED, may be used to determine the number
//! of bytes of data currently available in the receive buffer.
//!
//! \return Returns the number of available bytes.
//
//*****************************************************************************
uint8_t UARTRxBytesAvail(void)
{
    return(RX_BUFFER_USED);
}

//*****************************************************************************
//
//! Returns the number of bytes free in the transmit buffer.
//!
//! This function, available only when the module is built to operate in
//! buffered mode using \b UART_BUFFERED, may be used to determine the amount
//! of space currently available in the transmit buffer.
//!
//! \return Returns the number of free bytes.
//
//*****************************************************************************
uint8_t UARTTxBytesFree(void)
{
    return(TX_BUFFER_FREE);
}
