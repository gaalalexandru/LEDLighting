/*
 * uart_handler.h
 *
 * Created: 10/21/2017 10:17:19 PM
 *  Author: Gaal Alexandru
 */ 

#if 0

#ifndef UART_HANDLER_H_
#define UART_HANDLER_H_

#include <stdarg.h>

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
	
#define UART_BUFFERED (1)
//*****************************************************************************
//
// If built for buffered operation, the following labels define the sizes of
// the transmit and receive buffers respectively.
//
//*****************************************************************************
#ifdef UART_BUFFERED
#ifndef UART_RX_BUFFER_SIZE
	#define UART_RX_BUFFER_SIZE		2048 //AleGaa increased butter size, it was: 128
#endif
#ifndef UART_TX_BUFFER_SIZE
	#define UART_TX_BUFFER_SIZE		2048 //AleGaa increased butter size, it was: 1024
#endif
#endif


//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void UARTStdioConfig(uint32_t ui32Port, uint32_t ui32Baud,
                            uint32_t ui32SrcClock);
extern int UARTgets(char *pcBuf, uint32_t ui32Len);
extern unsigned char UARTgetc(void);
extern void UARTprintf(const char *pcString, ...);
extern void UARTvprintf(const char *pcString, va_list vaArgP);
extern int UARTwrite(const char *pcBuf, uint32_t ui32Len);
#ifdef UART_BUFFERED
extern int UARTPeek(unsigned char ucChar);
extern void UARTFlushTx(bool bDiscard);
extern void UARTFlushRx(void);
extern int UARTRxBytesAvail(void);
extern int UARTTxBytesFree(void);
extern void UARTEchoSet(bool bEnable);
#endif

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* UART_HANDLER_H_ */

#endif