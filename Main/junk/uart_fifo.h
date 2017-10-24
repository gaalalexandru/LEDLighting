/*
 * uart_fifo.h
 *
 * Created: 10/22/2017 1:33:53 AM
 *  Author: Gaal Alexandru
 */ 


#ifndef UART_FIFO_H_
#define UART_FIFO_H_

void UARTFlushRx(void);
void UARTFlushTx(bool bDiscard);
int16_t UARTPeek(uint8_t ucChar);
uint8_t UARTRxBytesAvail(void);
uint8_t UARTTxBytesFree(void);


#endif /* UART_FIFO_H_ */