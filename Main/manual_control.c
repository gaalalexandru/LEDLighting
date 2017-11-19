/*
 * manual_control.c
 *
 * Created: 10/26/2017 12:41:46 AM
 *  Author: Gaal Alexandru
 */

#include <avr/io.h>
#include <ctype.h>
#include "manual_control.h"
#include "configuration.h"
#include "uart_handler.h"
#include "pwm_handler.h"

/************************************************************************/
/*                           Global variables                           */
/************************************************************************/
extern volatile uint8_t pwm_width_buffer[CHMAX];

/************************************************************************/
/*                         Function definitions                         */
/************************************************************************/
static uint8_t get_next_char(void)
{
	int8_t tmp;
	tmp = uart_get_char(); // get next character
	tmp = toupper(tmp);     // force to upper case
	return (tmp);
}


//can't use manual control if wifi module is turned on
void manual_control(void)
{
	unsigned char rxdata, channel, temp, error, i;

	error = 0; // clear error flag
	while (uart_get_char() != '#')
	;                              // wait for sync character
	uart_send_char('#');              // echo sync character
	channel = uart_get_char() - 0x30; // receive channel number
	if (channel >= CHMAX)
	error = 1;                   // error if invalid channel
	uart_send_char(channel + 0x30); // echo received character

	temp = get_next_char();  // fetch upper nibble
	uart_send_char(temp); // echo received character
	if (isxdigit(temp))    // check for a hex character
	{
		if (temp > '9')
		temp -= 7; // subtract offset for A-F
		temp -= 0x30;  // subtract ASCII offset
	} else
	error = 1;         // error if not hex
	rxdata    = temp << 4; // store received upper nibble

	temp = get_next_char();  // fetch lower nibble
	uart_send_char(temp); // echo received character
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

		uart_send_char(':'); // send OK message
		uart_send_char('O');
		uart_send_char('K');
		uart_send_char('\r');
		uart_send_char('\n');
	} else // if data is not good
	{
		uart_send_char(':'); // send ERRor message
		uart_send_char('E');
		uart_send_char('R');
		uart_send_char('R');
		uart_send_char('\r');
		uart_send_char('\n');
	}
	uart_send_string("PWM values are: ");
	uart_newline();
	for(i=0;i<CHMAX;i++){
		uart_send_string("Channel: ");
		uart_send_udec(i);
		uart_send_string(" = ");
		uart_send_uhex(pwm_width_buffer[i]);
		uart_newline();
	}
	
	uart_send_string("PORT B = ");
	uart_send_uhex(PORTB);
	uart_newline();
	uart_send_string("PORT D = ");
	uart_send_uhex(PORTD);
	uart_newline();
}