//*************************************************************************************
/** \file uart.c
 *  \brief This file contains functions which interface with the Atmega UART.
 *
 *  Revisions:
 *    \li 04-01-2014 created original file
 *
 *  License:
 *		This file is copyright 2012 by Jonathan Fish and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * 		IMPLIED 	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdlib.h> 
#include <avr/io.h> 
#include "uart.h"
#include <math.h>
#include <stdio.h>

//-------------------------------------------------------------------------------------
/** \brief This Function initializes the UART.
 */
void usart_init(void)
{
	uint32_t ubrr = (CLK_SPEED/16UL)/BAUD-1; // calculates value for Baud Rate register
	UBRR0H = (unsigned char)(ubrr>>8);       // based on 9600 baud rate and 16MHz clock speed.
	UBRR0L = (unsigned char)ubrr;            // assigns this vallue to the baud rate register.
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);          // enable transmit and receive
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);         // configure for 1 stop bit, with an 8 character data packet.
	UCSR0A &= ~(1<<U2X0);                    // ensure double uart speed is cleared. 
}

//-------------------------------------------------------------------------------------
/** \brief This Function prompts the UART to transmit a byte.
 */
void usart_send( uint8_t data )
{
	while ( !( UCSR0A & (1<<UDRE0)) ); // Wait for empty transmit buffer
	UDR0 = data;                       // Put data into buffer, sends the data 
	return;
}

//-------------------------------------------------------------------------------------
/** \brief This Function prompts the UART to recieve a byte.
 *  @return Returns recieved byte of data.
 */
uint8_t usart_recv(void)
{
	while ( !(UCSR0A & (1<<RXC0)) ); // Wait for data to be received
	return UDR0;                     // Get and return received data from buffer
}

//-------------------------------------------------------------------------------------
/** \brief This Function returns true when there is new data on the uart
 * bus
 */
uint8_t  usart_istheredata(void)
{
	 return (UCSR0A & (1<<RXC0));
}




