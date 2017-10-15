/*
 * usart_handler.c
 *
 * Created: 10/4/2017 10:17:44 PM
 *  Author: Gaal Alexandru
 */ 

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 4.12, Section 4.9.4, Figures 4.26 and 4.40

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <avr/io.h>
#include "usart_handler.h"
#include "configuration.h"
#include "pwm_handler.h"
#include <ctype.h>

#define USART0_Transmit	USART_OutChar
#define USART0_Receive	USART_InChar

extern volatile uint8_t pwm_width_buffer[CHMAX];

static unsigned char get_next_char(void)
{
	char tmp;
	tmp = USART0_Receive(); // get next character
	tmp = toupper(tmp);     // force to upper case
	return (tmp);
}

void usart_manual_control(void)
{
	unsigned char rxdata, channel, temp, error, i;

	error = 0; // clear error flag
	while (USART0_Receive() != '#')
	;                              // wait for sync character
	USART0_Transmit('#');              // echo sync character
	channel = USART0_Receive() - 0x30; // receive channel number
	if (channel >= CHMAX)
	error = 1;                   // error if invalid channel
	USART0_Transmit(channel + 0x30); // echo received character

	temp = get_next_char();  // fetch upper nibble
	USART0_Transmit(temp); // echo received character
	if (isxdigit(temp))    // check for a hex character
	{
		if (temp > '9')
		temp -= 7; // subtract offset for A-F
		temp -= 0x30;  // subtract ASCII offset
	} else
	error = 1;         // error if not hex
	rxdata    = temp << 4; // store received upper nibble

	temp = get_next_char();  // fetch lower nibble
	USART0_Transmit(temp); // echo received character
	if (isxdigit(temp))    // check for a hex character
	{
		if (temp > '9')
		temp -= 7; // subtract offset for A-F
		temp -= 0x30;  // subtract ASCII offset
	} else
	error = 1;  // error if not hex
	rxdata += temp; // add lower nibble to upper nibble

	if (!error) // if data is good
	{
		pwm_width_buffer[channel] = rxdata; // update pwm_width buffer

		USART0_Transmit(':'); // send OK message
		USART0_Transmit('O');
		USART0_Transmit('K');
		USART0_Transmit('\r');
		USART0_Transmit('\n');
	} else // if data is not good
	{
		USART0_Transmit(':'); // send ERRor message
		USART0_Transmit('E');
		USART0_Transmit('R');
		USART0_Transmit('R');
		USART0_Transmit('\r');
		USART0_Transmit('\n');
	}
	USART_OutString("PWM values are: ");
	USART_NewLine();
	for(i=0;i<CHMAX;i++){
		USART_OutString("Channel: ");
		USART_OutUDec(i);
		USART_OutString(" = ");
		USART_OutUHex(pwm_width_buffer[i]);
		USART_NewLine();
	}
	/*
	USART_OutString("PORT B = ");
	USART_OutUHex(PORTB);
	USART_NewLine();
	USART_OutString("PORT D = ");
	USART_OutUHex(PORTD);
	USART_NewLine();
	*/
}

/* USART Initialization function*/
void USART_Init(uint32_t ubrr)
{
	/* Set baud rate */
	UBRRH = (uint8_t)(ubrr>>8);
	UBRRL = (uint8_t)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

//---------------------USART_NewLine---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void USART_NewLine(void){
  USART_OutChar(CR);
  USART_OutChar(LF);
}
/* **************************************** Output functions **************************************** */
//------------USART_OutChar------------
// Output 8-bit to serial port
// Input: data is an 8-bit ASCII character to be transferred
// Output: none
void USART_OutChar( uint8_t data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
	;
	/* Put data into buffer, sends the data */
	UDR = data;
}

//------------USART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void USART_OutString(char *pt){
  while(*pt){
    USART_OutChar(*pt);
    pt++;
  }
}

//-----------------------USART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void USART_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    USART_OutUDec(n/10);
    n = n%10;
  }
  USART_OutChar(n+'0'); /* n is between 0 and 9 */
}

//--------------------------USART_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void USART_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    USART_OutUHex(number/0x10);
    USART_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      USART_OutChar(number+'0');
     }
    else{
      USART_OutChar((number-0x0A)+'A');
    }
  }
}
/* **************************************** Input functions **************************************** */
//------------USART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
uint8_t USART_InChar( void )
{
	/* Wait for data to be received */
	while ( !(UCSRA & (1<<RXC)) )
	;
	/* Get and return received data from buffer */
	return UDR;
}

//------------USART_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void USART_InString(char *bufPt, uint16_t max) {
int length=0;
char character;
  character = USART_InChar();
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        USART_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      USART_OutChar(character);
    }
    character = USART_InChar();
  }
  *bufPt = 0;
}

//---------------------USART_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 32-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 8 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFFFFFF
// If you enter a number above FFFFFFFF, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t USART_InUHex(void){
uint32_t number=0, digit, length=0;
char character;
  character = USART_InChar();
  while(character != CR){
    digit = 0x10; // assume bad
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit <= 0xF){
      number = number*0x10+digit;
      length++;
      USART_OutChar(character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if((character==BS) && length){
      number /= 0x10;
      length--;
      USART_OutChar(character);
    }
    character = USART_InChar();
  }
  return number;
}